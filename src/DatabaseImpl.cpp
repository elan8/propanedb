#include <filesystem>
namespace fs = std::filesystem;
#include "rocksdb/db.h"
#include "rocksdb/utilities/backupable_db.h"

#include "DatabaseImpl.hpp"

DatabaseImpl::DatabaseImpl(const string &path, bool debug) : databasePath(path), backupPath("/tmp/rocksdb_backup")
{
  descriptorDB = new google::protobuf::SimpleDescriptorDatabase();
  pool = new google::protobuf::DescriptorPool(descriptorDB);
  queryParser = new QueryParser(debug);
  this->debug = debug;
}

DatabaseImpl::~DatabaseImpl()
{
  for (auto it = databases.begin(); it != databases.end(); ++it)
  {
    DB *db = it->second;
    db->Close();
  }

  delete descriptorDB;
}

void DatabaseImpl::setDebugMode(bool enabled)
{
  debug = enabled;
}

grpc::Status DatabaseImpl::CreateDatabase(Metadata *metadata, const propane::PropaneDatabase *request,
                                          propane::PropaneStatus *reply)
{

  DB *db;
  string databaseName = request->databasename();
  if (databaseName.length() == 0)
  {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }
  if (debug)
  {
    LOG(INFO) << "CreateDatabase databaseName=" << databaseName << endl;
  }

  fs::path p1;
  p1 += databasePath;
  p1 /= databaseName;
  string path = p1.generic_string();

  google::protobuf::FileDescriptorSet fds = request->descriptor_set();
  descriptorDB = new google::protobuf::SimpleDescriptorDatabase();

  auto file = fds.file();
  for (auto it = file.begin(); it != file.end(); ++it)
  {
    descriptorDB->Add((*it));
  }
  pool = new google::protobuf::DescriptorPool(descriptorDB);

  db = databases[databaseName];
  bool databaseExists = fs::exists(path);
  if (db != nullptr || databaseExists)
  {
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "A database with this name already exists");
  }

  Options options;
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  options.create_if_missing = true;
  ROCKSDB_NAMESPACE::Status s = DB::Open(options, path, &db);
  if (debug)
  {
    LOG(INFO) << "Status code="s << s.ToString() << endl;
  }
  assert(s.ok());

  databases[databaseName] = db;
  return grpc::Status::OK;
}

rocksdb::DB *DatabaseImpl::GetDatabase(string name)
{
  if (debug)
  {
    LOG(INFO) << "GetDatabase" << endl;
  }
  fs::path p1;
  p1 += databasePath;
  p1 /= name;
  string path = p1.generic_string();

  if (name.length() == 0)
  {
    LOG(ERROR) << "Error: Database name=empty" << endl;
    return 0;
  }

  DB *db = databases[name];
  if (db == nullptr)
  {
    if (debug)
    {
      LOG(INFO) << "Database pointer = null: opening database" << endl;
    }
    Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    ROCKSDB_NAMESPACE::Status s = DB::Open(options, path, &db);
    if (debug)
    {
      LOG(INFO) << "DB Open: Status code="s << s.ToString() << endl;
    }
    assert(s.ok());
    databases[name] = db;
  }

  return db;
}

grpc::Status DatabaseImpl::Put(Metadata *metadata, const propane::PropanePut *request,
                               propane::PropaneId *reply)
{
  if (debug)
  {
    LOG(INFO) << "Put" << endl;
  }
  string databaseName = metadata->databaseName;
  if (databaseName.length() == 0)
  {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }

  Any any = (request->entity()).data();
  string typeUrl = any.type_url();
  string typeName = Util::getTypeName(typeUrl);
  const google::protobuf::Descriptor *descriptor = pool->FindMessageTypeByName(typeName);
  if (descriptor == nullptr)
  {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Descriptor with this type was not found");
  }
  google::protobuf::Message *message = dmf.GetPrototype(descriptor)->New();

  if (!any.UnpackTo(message))
  {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Unpack of Any to message failed. Check if the type of this object was registered in the FileDescriptorSet.");
  }

  const google::protobuf::FieldDescriptor *fd = descriptor->FindFieldByName("id");
  const google::protobuf::Reflection *reflection = message->GetReflection();

  string id = reflection->GetString(*message, fd);
  if (id.length() == 0)
  {
    id = Util::generateUUID();
    reflection->SetString(message, fd, id);
    any.PackFrom(*message);
  }
  if (debug)
  {
    LOG(INFO) << "Entity= " << any.DebugString() << endl;
  }

  string serializedAny;
  any.SerializeToString(&serializedAny);

  ROCKSDB_NAMESPACE::Status s = GetDatabase(databaseName)->Put(WriteOptions(), id, serializedAny);
  assert(s.ok());
  reply->set_id(id);

  return grpc::Status::OK;
}

grpc::Status DatabaseImpl::Get(Metadata *metadata, const propane::PropaneId *request,
                               propane::PropaneEntity *reply)
{
  if (debug)
  {
    LOG(INFO) << "Get" << endl;
  }
  string databaseName = metadata->databaseName;
  if (databaseName.length() == 0)
  {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }

  string serializedAny;
  ROCKSDB_NAMESPACE::Status s = GetDatabase(databaseName)->Get(ReadOptions(), request->id(), &serializedAny);
  google::protobuf::Any *any = new Any();
  any->ParseFromString(serializedAny);
  reply->set_allocated_data(any);
  return grpc::Status::OK;
}

grpc::Status DatabaseImpl::Delete(Metadata *metadata, const propane::PropaneId *request,
                                  propane::PropaneStatus *reply)
{
  string databaseName = metadata->databaseName;
  if (databaseName.length() == 0)
  {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }

  string serializedAny;
  ROCKSDB_NAMESPACE::Status s = GetDatabase(databaseName)->Get(ReadOptions(), request->id(), &serializedAny);
  if (s.ok())
  {
    ROCKSDB_NAMESPACE::Status s = GetDatabase(databaseName)->Delete(WriteOptions(), request->id());
    LOG(INFO) << "Delete status= " << s.ToString() << endl;
    if (!s.ok())
    {
      return grpc::Status(grpc::StatusCode::ABORTED, "Could not delete object");
    }
  }
  else
  {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "No object with this ID in database");
  }
  return grpc::Status::OK;
}

grpc::Status DatabaseImpl::Search(Metadata *metadata, const propane::PropaneSearch *request,
                                  propane::PropaneEntities *reply)
{
  if (debug)
  {
    LOG(INFO) << "Search" << endl;
  }
  string databaseName = metadata->databaseName;
  if (databaseName.length() == 0)
  {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }

  // request->entitytype()
  // Any any = (request->entity()).data();
  // string typeUrl = any.type_url();
  // string typeName = Util::getTypeName(typeUrl);
  const google::protobuf::Descriptor *descriptor = pool->FindMessageTypeByName(request->entitytype());
  if (descriptor == nullptr)
  {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Descriptor not found");
  }

  ROCKSDB_NAMESPACE::Iterator *it = GetDatabase(databaseName)->NewIterator(ReadOptions());

  Query query = queryParser->parseQuery(request->query());
  if (query.hasError())
  {
    LOG(ERROR) << "Query error: =: " << query.getErrorMessage() << std::endl;
    return grpc::Status(grpc::StatusCode::INTERNAL, "Query error:" + query.getErrorMessage());
  }
  reply->clear_entities();
  google::protobuf::RepeatedPtrField<::propane::PropaneEntity> *entities = reply->mutable_entities();
  for (it->Seek(""); it->Valid(); it->Next())
  {
    google::protobuf::Any *any = new Any();
    any->ParseFromString(it->value().ToString());
    string typeUrl = any->type_url();
    string typeName = Util::getTypeName(typeUrl);
    if (IsCorrectEntityType(any, request->entitytype()))
    {
      const google::protobuf::Descriptor *descriptor = pool->FindMessageTypeByName(typeName);
      if (descriptor != nullptr)
      {
        google::protobuf::Message *message = dmf.GetPrototype(descriptor)->New();
        any->UnpackTo(message);
        if (query.isMatch(descriptor, message))
        {
          propane::PropaneEntity *entity = entities->Add();
          entity->set_allocated_data(any);
          if (debug)
          {
            LOG(INFO) << "Search:: Add entity" << endl;
          }
        }
      }
    }
  }
  return grpc::Status::OK;
}

bool DatabaseImpl::IsCorrectEntityType(google::protobuf::Any *any, std::string entityType)
{
  bool output = false;
  string typeUrl = any->type_url();
  string typeName = Util::getTypeName(typeUrl);
  if (entityType.compare(typeName) == 0)
  {
    output = true;
  }
  return output;
}

grpc::Status DatabaseImpl::Backup(Metadata *metadata, const string databaseName, string *zipFilePath)
{


  BackupEngine *backup_engine;
  backup_engine->PurgeOldBackups(0);
  rocksdb::Status s = BackupEngine::Open(Env::Default(), BackupableDBOptions(backupPath), &backup_engine);
  assert(s.ok());
  rocksdb::DB *db = GetDatabase(databaseName);

  s = backup_engine->CreateNewBackup(db);
  assert(s.ok());

  std::vector<BackupInfo> backup_info;
  backup_engine->GetBackupInfo(&backup_info, true);

  LOG(INFO) << "Backup: file name = " << backup_info.front().file_details.front().relative_filename << endl;



  delete backup_engine;
  //TODO: write to ZIP file

  return grpc::Status::OK;
}

grpc::Status DatabaseImpl::Restore(Metadata *metadata, const string databaseName, string zipFilePath)
{
  //TODO: unzip file to RocksDB backup path

  BackupEngineReadOnly *backup_engine;
  rocksdb::Status s = BackupEngineReadOnly::Open(Env::Default(), BackupableDBOptions(backupPath), &backup_engine);
  assert(s.ok());
  s = backup_engine->RestoreDBFromBackup(1 , databasePath, databasePath);
  assert(s.ok());
  delete backup_engine;

  return grpc::Status::OK;
}