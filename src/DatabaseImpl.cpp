#include <filesystem>
#include <fstream>
#include <thread>
namespace fs = std::filesystem;
#include "rocksdb/db.h"
#include "rocksdb/utilities/backupable_db.h"
#include "Poco/Zip/Compress.h"
#include "Poco/Zip/Decompress.h"

#include "Poco/Delegate.h"

#include "DatabaseImpl.hpp"

DatabaseImpl::DatabaseImpl(const string &databasePath, const string &backupPath, bool debug) : databasePath(databasePath), backupPath(backupPath), databases()
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

void DatabaseImpl::CloseDatabases()
{
  if (debug)
  {
    LOG(INFO) << "CloseDatabases" << endl;
  }

  for (auto it = databases.begin(); it != databases.end(); ++it)
  {
    DB *db = it->second;
    auto status = db->Close();
    if (!status.ok())
    {
      LOG(FATAL) << "Error: " << status.ToString() << endl;
    }
    databases.erase(it->first);
  }
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
  if (fds.file_size() == 0)
  {
    LOG(ERROR) << "FileDescriptorSet is empty" << endl;
    return grpc::Status(grpc::StatusCode::INTERNAL, "FileDescriptorSet is empty");
  }

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
    LOG(ERROR) << "Database exists:" << path << endl;
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
    LOG(INFO) << "GetDatabase: " << name << endl;
  }
  fs::path p1;
  p1 += databasePath;
  p1 /= name;
  string path = p1.generic_string();
  LOG(INFO) << "Database path = " << path << endl;

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
    ROCKSDB_NAMESPACE::Status status = DB::Open(options, path, &db);
    if (!status.ok())
    {
      LOG(ERROR) << "Error: Status = " << status.ToString() << endl;
      return 0;
    }

    databases[name] = db;
  }
  else
  {
    if (debug)
    {
      LOG(INFO) << "Re use database pointer " << endl;
    }
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
    LOG(INFO) << "Get: " << endl;
  }
  string databaseName = metadata->databaseName;
  if (databaseName.length() == 0)
  {
    LOG(INFO) << "Database name is empty" << endl;
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }

  LOG(INFO) << "Get: print all entities" << endl;
  rocksdb::DB *db = GetDatabase(databaseName);

  ROCKSDB_NAMESPACE::Iterator *it = db->NewIterator(ReadOptions());

  google::protobuf::Any *any1 = new Any();
  for (it->Seek(""); it->Valid(); it->Next())
  {
    any1->ParseFromString(it->value().ToString());
    LOG(INFO) << "Get: entity key = " << it->key().ToString() << endl;
    LOG(INFO) << "Get: entity debug string = " << any1->DebugString() << endl;
  }

  //   rocksdb::DB *db = GetDatabase(databaseName);
  //   ROCKSDB_NAMESPACE::Iterator *it = db->NewIterator(ReadOptions());
  //   google::protobuf::Any *any1 = new Any();
  //   for (it->Seek(""); it->Valid(); it->Next())
  //   {
  //     any1->ParseFromString(it->value().ToString());
  //     LOG(INFO) << "Get: entity key = " << it->key().ToString() << endl;
  //     LOG(INFO) << "Get: entity debug string = " << any1->DebugString() << endl;
  //   }

  LOG(INFO) << "Get: request ID= " << request->id() << endl;

  string serializedAny;
  ROCKSDB_NAMESPACE::Status s = GetDatabase(databaseName)->Get(ReadOptions(), request->id(), &serializedAny);
  if (!s.ok())
  {
    LOG(INFO) << "Error:" << s.ToString() << endl;
    return grpc::Status(grpc::StatusCode::INTERNAL, s.ToString());
  }
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
    LOG(INFO) << "Delete status= " << s.ToString() << endl;
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

grpc::Status DatabaseImpl::Backup(Metadata *metadata, const string databaseName, string zipFilePath)
{

  BackupEngine *backup_engine;

  auto options = BackupableDBOptions(backupPath);
  options.share_table_files = false;

  rocksdb::Status s = BackupEngine::Open(Env::Default(), options, &backup_engine);
  assert(s.ok());
  backup_engine->PurgeOldBackups(0);

  rocksdb::DB *db = GetDatabase(databaseName);

  ROCKSDB_NAMESPACE::Iterator *it = db->NewIterator(ReadOptions());

  google::protobuf::Any *any = new Any();
  for (it->Seek(""); it->Valid(); it->Next())
  {
    any->ParseFromString(it->value().ToString());
    LOG(INFO) << "Backup: entity key = " << it->key().ToString() << endl;
    LOG(INFO) << "Backup: entity debug string = " << any->DebugString() << endl;
  }

  s = backup_engine->CreateNewBackup(db, true);
  //assert(s.ok());
  if (!s.ok())
  {
    LOG(INFO) << "CreateNewBackup error= " << s.ToString() << endl;
  }

  std::vector<BackupInfo> backup_info;
  backup_engine->GetBackupInfo(&backup_info, true);

  LOG(INFO) << "Backup: file name = " << backup_info.front().file_details.front().relative_filename << endl;
  LOG(INFO) << "Backup: backup ID = " << backup_info.front().backup_id << endl;

  s = backup_engine->VerifyBackup(backup_info.front().backup_id);
  //assert(s.ok());
  if (!s.ok())
  {
    LOG(ERROR) << "VerifyBackup error= " << s.ToString() << endl;
  }

  std::ofstream out(zipFilePath, std::ios::binary);
  Poco::Zip::Compress c(out, false);
  Poco::Path backupDir(backupPath);
  c.addRecursive(backupDir);
  c.close(); // MUST be done to finalize the Zip file

  backup_engine->PurgeOldBackups(0);
  delete backup_engine;

  return grpc::Status::OK;
}

void DatabaseImpl::onDecompressError(const void *pSender, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> &info)
{
  LOG(FATAL) << "Decompress error:" << std::endl;
}

grpc::Status DatabaseImpl::Restore(Metadata *metadata, const string databaseName, string zipFilePath)
{
  CloseDatabases();

  std::ifstream inp(zipFilePath, std::ios::binary);
  poco_assert(inp);
  // decompress to current working dir
  LOG(INFO) << "Restore: backupPath=" << backupPath;
  Poco::Zip::Decompress dec(inp, Poco::Path(backupPath));
  // if an error happens invoke the ZipTest::onDecompressError method
  dec.EError += Poco::Delegate<DatabaseImpl, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>>(this, &DatabaseImpl::onDecompressError);
  dec.decompressAllFiles();
  dec.EError -= Poco::Delegate<DatabaseImpl, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>>(this, &DatabaseImpl::onDecompressError);

  LOG(INFO) << "Restore: Create backup engine";
  LOG(INFO) << "Restore from" << backupPath;
  BackupEngineReadOnly *backup_engine;
  rocksdb::Status s = BackupEngineReadOnly::Open(Env::Default(), BackupableDBOptions(backupPath), &backup_engine);
  if (!s.ok())
  {
    LOG(FATAL) << "Error:" << s.ToString();
  }
  BackupInfo info;
  s = backup_engine->GetLatestBackupInfo(&info, false);
  if (s.IsNotFound())
  {
    LOG(INFO) << "No backup found, so nothing to restore";
  }
  else
  {
    s = backup_engine->RestoreDBFromLatestBackup(databaseName, databaseName);
    if (!s.ok())
    {
      LOG(FATAL) << "Error: " << s.ToString();
      return grpc::Status::CANCELLED;
    }
  }
  delete backup_engine;

  return grpc::Status::OK;
}