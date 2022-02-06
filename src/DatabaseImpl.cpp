#include <filesystem>
#include <fstream>
#include <thread>
namespace fs = std::filesystem;
#include "Poco/Zip/Compress.h"
#include "Poco/Zip/Decompress.h"
#include "rocksdb/db.h"
#include "rocksdb/utilities/backupable_db.h"

#include <boost/filesystem.hpp>

#include "Poco/Delegate.h"

#include "DatabaseImpl.hpp"

DatabaseImpl::DatabaseImpl(const string &databasePath, const string &backupPath,
                           bool debug)
    : databasePath(databasePath),
      backupPath(backupPath),
      databaseListFilename(
          Util::getAbsolutePath(databasePath, "databases.dat")),
      databases(),
      databaseList() {
  queryParser = new QueryParser(debug);
  this->debug = debug;

  ReadDatabaseList();
}

DatabaseImpl::~DatabaseImpl() {
  WriteDatabaseList();
  for (auto it = databases.begin(); it != databases.end(); ++it) {
    DB *db = it->second;
    db->Close();
    delete db;
  }
}

bool DatabaseImpl::ReadDatabaseList() {
  try {
    if (Util::pathExists(databaseListFilename)) {
      ifstream inputStream;
      inputStream.open(databaseListFilename);
      databaseList.ParseFromIstream(&inputStream);
      inputStream.close();
    }
  } catch (boost::filesystem::filesystem_error const &e) {
    LOG(INFO) << "Error: " << e.what() << std::endl;
  }
  return true;
}
bool DatabaseImpl::WriteDatabaseList() {
  ofstream outputStream;
  outputStream.open(databaseListFilename);
  databaseList.SerializeToOstream(&outputStream);
  outputStream.close();
  return true;
}

propane::PropaneDatabase *DatabaseImpl::AddDatabaseToList(
    propane::PropaneDatabase entry) {
  string id = Util::generateUUID();
  auto db = databaseList.mutable_databases()->Add();
  db->set_id(id);
  db->set_name(entry.name());
  auto fds = new google::protobuf::FileDescriptorSet();
  fds->CopyFrom(entry.descriptor_set());
  db->set_allocated_descriptor_set(fds);
  WriteDatabaseList();
  return db;
}

bool DatabaseImpl::FindDatabaseInList(string databaseName,
                                      propane::PropaneDatabase &output) {
  for (int i = 0; i < databaseList.databases_size(); i++) {
    auto db = databaseList.databases().at(i);
    if (db.name().compare(databaseName) == 0) {
      output = db;
      return true;
    }
  }
  return false;
}
bool DatabaseImpl::UpdateDatabaseInList(propane::PropaneDatabase entry) {
  for (int i = 0; i < databaseList.databases_size(); i++) {
    auto db = databaseList.databases().at(i);
    if (db.id().compare(entry.id()) == 0) {
      auto mdb = databaseList.mutable_databases()->Mutable(i);
      if (db.name().length() > 0) {
        mdb->set_name(entry.name());
      }
      if (db.has_descriptor_set()) {
        google::protobuf::FileDescriptorSet *fds =
            new google::protobuf::FileDescriptorSet(entry.descriptor_set());
        mdb->set_allocated_descriptor_set(fds);
      }
      return true;
    }
  }
  return false;
}

bool DatabaseImpl::DeleteDatabaseFromList(string uuid) {
  for (int i = 0; i < databaseList.databases_size(); i++) {
    auto db = databaseList.databases().at(i);
    if (db.id().compare(uuid) == 0) {
      databaseList.mutable_databases()->DeleteSubrange(i, 1);
      return true;
    }
  }
  return false;
}

google::protobuf::DescriptorPool DatabaseImpl::GetDescriptorPool(
    propane::PropaneDatabase database) {
  auto descriptorDB = new google::protobuf::SimpleDescriptorDatabase();
  auto fds = database.descriptor_set();
  if (fds.file_size() > 0) {
    auto file = fds.file();
    for (auto it = file.begin(); it != file.end(); ++it) {
      descriptorDB->Add((*it));
    }
    return google::protobuf::DescriptorPool(descriptorDB);
  }
  return google::protobuf::DescriptorPool();
}

void DatabaseImpl::setDebugMode(bool enabled) { debug = enabled; }

void DatabaseImpl::CloseDatabases() {
  if (debug) {
    LOG(INFO) << "CloseDatabases" << endl;
  }

  for (auto it = databases.begin(); it != databases.end(); ++it) {
    DB *db = it->second;
    auto status = db->Close();
    if (!status.ok()) {
      LOG(FATAL) << "Error: " << status.ToString() << endl;
    }
    databases.erase(it->first);
  }
}

grpc::Status DatabaseImpl::UpdateDatabase(
    Metadata *metadata, const propane::PropaneDatabaseRequest *request,
    propane::PropaneStatus *reply) {
  databaseModifyMutex.lock();
  bool updateDatabase = false;
  DB *db;
  google::protobuf::DescriptorPool *pool;
  string databaseName = request->databasename();
  string newDatabaseName = request->newdatabasename();

  if (databaseName.length() == 0) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }
  if (debug) {
    LOG(INFO) << "UpdateDatabase databaseName=" << databaseName << endl;
    LOG(INFO) << "UpdateDatabase newDatabaseName=" << newDatabaseName << endl;
  }

  auto pdb = propane::PropaneDatabase();
  if (!FindDatabaseInList(databaseName, pdb)) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database not found");
  }

  string path = Util::getAbsolutePath(databasePath, pdb.id());
  if (debug) {
    LOG(INFO) << "UpdateDatabase databaseName=" << databaseName << endl;
    LOG(INFO) << "UpdateDatabase path=" << path << endl;
  }

  if (request->descriptor_set().file_size() > 0) {
    if (debug) {
      LOG(INFO) << "Update file descriptor set" << endl;
    }
    google::protobuf::FileDescriptorSet *fds =
        new google::protobuf::FileDescriptorSet(request->descriptor_set());
    pdb.set_allocated_descriptor_set(fds);
    updateDatabase = true;
  }

  if (newDatabaseName.length() > 0) {
    if (debug) {
      LOG(INFO) << "Rename database to " << newDatabaseName << endl;
    }
    pdb.set_name(newDatabaseName);
    updateDatabase = true;
  }
  if (updateDatabase) {
    if (!UpdateDatabaseInList(pdb)) {
      return grpc::Status(grpc::StatusCode::INTERNAL,
                          "Update of database failed");
    }
    WriteDatabaseList();
  }
  databaseModifyMutex.unlock();
  return grpc::Status::OK;
}

grpc::Status DatabaseImpl::DeleteDatabase(
    Metadata *metadata, const propane::PropaneDatabaseRequest *request,
    propane::PropaneStatus *reply) {
  databaseModifyMutex.lock();
  DB *db;
  string databaseName = request->databasename();
  if (databaseName.length() == 0) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }

  auto pdb = propane::PropaneDatabase();
  if (!FindDatabaseInList(databaseName, pdb)) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database not found");
  }
  string path = Util::getAbsolutePath(databasePath, pdb.id());

  if (debug) {
    LOG(INFO) << "DeleteDatabase databaseName=" << databaseName << endl;
    LOG(INFO) << "DeleteDatabase path=" << path << endl;
  }

  bool databaseExists = Util::pathExists(path);
  if (!databaseExists) {
    LOG(ERROR) << "A database with this name does not exist:" << path << endl;
    return grpc::Status(grpc::StatusCode::NOT_FOUND,
                        "A database with this name does not exist");
  }
  db = GetDatabase(databaseName);
  if (db != nullptr) {
    db->Close();
  }
  try {
    if (Util::pathExists(path)) {
      boost::filesystem::remove_all(path);
    }
  } catch (boost::filesystem::filesystem_error const &e) {
    LOG(INFO) << "Error: " << e.what() << std::endl;
    return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
  }
  DeleteDatabaseFromList(pdb.id());
  WriteDatabaseList();
  databaseModifyMutex.unlock();

  return grpc::Status::OK;
}

grpc::Status DatabaseImpl::CreateDatabase(
    Metadata *metadata, const propane::PropaneDatabaseRequest *request,
    propane::PropaneStatus *reply) {
  databaseModifyMutex.lock();
  DB *db;

  string databaseName = request->databasename();
  if (databaseName.length() == 0) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }
  if (debug) {
    LOG(INFO) << "CreateDatabase databaseName=" << databaseName << endl;
  }

  auto pdb = propane::PropaneDatabase();
  pdb.set_name(databaseName);
  google::protobuf::FileDescriptorSet *fds =
      new google::protobuf::FileDescriptorSet(request->descriptor_set());
  pdb.set_allocated_descriptor_set(fds);
  auto entry = AddDatabaseToList(pdb);
  string path = Util::getAbsolutePath(databasePath, pdb.id());
  Options options;
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  options.create_if_missing = true;
  ROCKSDB_NAMESPACE::Status s = DB::Open(options, path, &db);
  if (debug) {
    LOG(INFO) << "Status code="s << s.ToString() << endl;
  }
  databases[entry->id()] = db;
  databaseModifyMutex.unlock();
  return grpc::Status::OK;
}

rocksdb::DB *DatabaseImpl::GetDatabase(string name) {
  DB *db;
  if (debug) {
    LOG(INFO) << "GetDatabase: " << name << endl;
  }
  string path = Util::getAbsolutePath(databasePath, name);
  if (debug) {
    LOG(INFO) << "Database path = " << path << endl;
  }
  if (name.length() == 0) {
    LOG(ERROR) << "Error: Database name=empty" << endl;
    return nullptr;
  }

  propane::PropaneDatabase selectedDatabase;
  if (!FindDatabaseInList(name, selectedDatabase)) {
    LOG(ERROR) << "Error: Database not found" << endl;
    return nullptr;
  }
  db = databases[selectedDatabase.id()];
  if (db == nullptr) {
    if (debug) {
      LOG(INFO) << "Database pointer = null: opening database" << endl;
    }
    Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    ROCKSDB_NAMESPACE::Status status = DB::Open(options, path, &db);
    if (!status.ok()) {
      LOG(ERROR) << "Error: Status = " << status.ToString() << endl;
      return nullptr;
    }
    databases[selectedDatabase.id()] = db;
  }
  return db;
}

grpc::Status DatabaseImpl::Put(Metadata *metadata,
                               const propane::PropanePut *request,
                               propane::PropaneId *reply) {
  if (debug) {
    LOG(INFO) << "Put" << endl;
  }
  string databaseName = metadata->databaseName;
  if (databaseName.length() == 0) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }

  Any any = (request->entity()).data();
  string typeUrl = any.type_url();
  string typeName = Util::getTypeName(typeUrl);

  propane::PropaneDatabase selectedDatabase;
  if (!FindDatabaseInList(databaseName, selectedDatabase)) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "Error: Database not found in list");
  }

  auto dp = GetDescriptorPool(selectedDatabase);
  const google::protobuf::Descriptor *descriptor =
      dp.FindMessageTypeByName(typeName);
  if (descriptor == nullptr) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND,
                        "Descriptor with this type was not found");
  }
  google::protobuf::Message *message = dmf.GetPrototype(descriptor)->New();

  if (!any.UnpackTo(message)) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "Unpack of Any to message failed. Check if the type of "
                        "this object was registered in the FileDescriptorSet.");
  }
  const google::protobuf::FieldDescriptor *fd =
      descriptor->FindFieldByName("id");
  const google::protobuf::Reflection *reflection = message->GetReflection();
  string id = reflection->GetString(*message, fd);
  if (id.length() == 0) {
    id = Util::generateUUID();
    reflection->SetString(message, fd, id);
    any.PackFrom(*message);
  }
  if (debug) {
    LOG(INFO) << "Entity= " << any.DebugString() << endl;
  }

  string serializedAny;
  any.SerializeToString(&serializedAny);

  rocksdb::DB *db = GetDatabase(databaseName);
  if (db == nullptr) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Cannot find database");
  }
  ROCKSDB_NAMESPACE::Status s = db->Put(WriteOptions(), id, serializedAny);
  reply->set_id(id);
  return grpc::Status::OK;
}

grpc::Status DatabaseImpl::Get(Metadata *metadata,
                               const propane::PropaneId *request,
                               propane::PropaneEntity *reply) {
  if (debug) {
    LOG(INFO) << "Get: request ID= " << request->id() << endl;
  }
  string databaseName = metadata->databaseName;
  if (databaseName.length() == 0) {
    LOG(INFO) << "Database name is empty" << endl;
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }

  rocksdb::DB *db = GetDatabase(databaseName);
  if (db == nullptr) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Cannot find database");
  }

  string serializedAny;
  ROCKSDB_NAMESPACE::Status s =
      db->Get(ReadOptions(), request->id(), &serializedAny);
  if (!s.ok()) {
    LOG(INFO) << "Error:" << s.ToString() << endl;
    return grpc::Status(grpc::StatusCode::INTERNAL, s.ToString());
  }
  google::protobuf::Any *any = new Any();
  any->ParseFromString(serializedAny);
  reply->set_allocated_data(any);
  return grpc::Status::OK;
}

grpc::Status DatabaseImpl::Delete(Metadata *metadata,
                                  const propane::PropaneId *request,
                                  propane::PropaneStatus *reply) {
  string databaseName = metadata->databaseName;
  if (databaseName.length() == 0) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }
  rocksdb::DB *db = GetDatabase(databaseName);
  if (db == nullptr) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Cannot find database");
  }

  string serializedAny;
  ROCKSDB_NAMESPACE::Status s =
      db->Get(ReadOptions(), request->id(), &serializedAny);
  if (s.ok()) {
    ROCKSDB_NAMESPACE::Status s =
        GetDatabase(databaseName)->Delete(WriteOptions(), request->id());
    if (debug) {
      LOG(INFO) << "Delete status= " << s.ToString() << endl;
    }
    if (!s.ok()) {
      return grpc::Status(grpc::StatusCode::ABORTED, "Could not delete object");
    }
  } else {
    if (debug) {
      LOG(INFO) << "Delete status= " << s.ToString() << endl;
    }
    return grpc::Status(grpc::StatusCode::NOT_FOUND,
                        "No object with this ID in database");
  }
  return grpc::Status::OK;
}

grpc::Status DatabaseImpl::Search(Metadata *metadata,
                                  const propane::PropaneSearch *request,
                                  propane::PropaneEntities *reply) {
  if (debug) {
    LOG(INFO) << "Search" << endl;
  }
  string databaseName = metadata->databaseName;
  if (databaseName.length() == 0) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Database name is empty");
  }

  propane::PropaneDatabase selectedDatabase;
  if (!FindDatabaseInList(databaseName, selectedDatabase)) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "Error: Database not found in list");
  }
  string requestTypeName = Util::getTypeName(request->entitytype());
  auto dp = GetDescriptorPool(selectedDatabase);
  const google::protobuf::Descriptor *descriptor =
      dp.FindMessageTypeByName(requestTypeName);
  if (descriptor == nullptr) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Descriptor not found");
  }

  rocksdb::DB *db = GetDatabase(databaseName);
  if (db == nullptr) {
    return grpc::Status(grpc::StatusCode::INTERNAL, "Cannot find database");
  }

  ROCKSDB_NAMESPACE::Iterator *it = db->NewIterator(ReadOptions());

  Query query = queryParser->parseQuery(request->query());
  if (query.hasError()) {
    LOG(ERROR) << "Query error: =: " << query.getErrorMessage() << std::endl;
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        "Query error:" + query.getErrorMessage());
  }
  if (debug) {
    LOG(INFO) << "query completed" << endl;
  }

  reply->clear_entities();
  google::protobuf::RepeatedPtrField<::propane::PropaneEntity> *entities =
      reply->mutable_entities();
  for (it->Seek(""); it->Valid(); it->Next()) {
    google::protobuf::Any *any = new Any();
    any->ParseFromString(it->value().ToString());
    string typeUrl = any->type_url();
    string typeName = Util::getTypeName(typeUrl);
    if (debug) {
      LOG(INFO) << "seek iteration: typeName=" << typeName << endl;
    }
    if (IsCorrectEntityType(any, requestTypeName)) {
      if (debug) {
        LOG(INFO) << "correct entity type" << endl;
      }
      const google::protobuf::Descriptor *descriptor =
          dp.FindMessageTypeByName(typeName);
      if (descriptor == nullptr) {
        return grpc::Status(grpc::StatusCode::INTERNAL,
                            "Descriptor null pointer:");
      }
      google::protobuf::Message *message = dmf.GetPrototype(descriptor)->New();
      any->UnpackTo(message);
      if (query.isMatch(descriptor, message)) {
        if (debug) {
          LOG(INFO) << "Query: isMatch" << endl;
        }
        propane::PropaneEntity *entity = entities->Add();
        entity->set_allocated_data(any);
        if (debug) {
          LOG(INFO) << "Search:: Add entity" << endl;
        }
      }
    }
  }
  return grpc::Status::OK;
}

bool DatabaseImpl::IsCorrectEntityType(google::protobuf::Any *any,
                                       std::string entityType) {
  bool output = false;
  string typeUrl = any->type_url();
  string typeName = Util::getTypeName(typeUrl);
  if (entityType.compare(typeName) == 0) {
    output = true;
  }
  return output;
}

grpc::Status DatabaseImpl::Backup(Metadata *metadata,
                                  const string &databaseName,
                                  const string &zipFilePath) {
  BackupEngine *backup_engine;

  auto options = BackupableDBOptions(backupPath);
  options.share_table_files = false;

  rocksdb::Status s =
      BackupEngine::Open(Env::Default(), options, &backup_engine);
  assert(s.ok());
  backup_engine->PurgeOldBackups(0);

  rocksdb::DB *db = GetDatabase(databaseName);
  s = backup_engine->CreateNewBackup(db, true);
  if (!s.ok()) {
    LOG(ERROR) << "CreateNewBackup error= " << s.ToString() << endl;
  }

  std::vector<BackupInfo> backup_info;
  backup_engine->GetBackupInfo(&backup_info, true);

  if (debug) {
    LOG(INFO) << "Backup: file name = "
              << backup_info.front().file_details.front().relative_filename
              << endl;
    LOG(INFO) << "Backup: backup ID = " << backup_info.front().backup_id
              << endl;
  }

  s = backup_engine->VerifyBackup(backup_info.front().backup_id);

  if (!s.ok()) {
    LOG(ERROR) << "VerifyBackup error= " << s.ToString() << endl;
  }

  std::ofstream out(zipFilePath, std::ios::binary);
  Poco::Zip::Compress c(out, false);
  Poco::Path backupDir(backupPath);
  c.addRecursive(backupDir);
  c.close();  // MUST be done to finalize the Zip file

  backup_engine->PurgeOldBackups(0);
  delete backup_engine;

  return grpc::Status::OK;
}

void DatabaseImpl::onDecompressError(
    const void *pSender,
    std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string> &info) {
  LOG(FATAL) << "Decompress error:" << std::endl;
}

grpc::Status DatabaseImpl::Restore(Metadata *metadata,
                                   const string &databaseName,
                                   const string &zipFilePath) {
  CloseDatabases();
  std::ifstream inp(zipFilePath, std::ios::binary);
  poco_assert(inp);
  Poco::Zip::Decompress dec(inp, Poco::Path(backupPath));
  // if an error happens invoke the ZipTest::onDecompressError method
  dec.EError += Poco::Delegate<
      DatabaseImpl,
      std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>>(
      this, &DatabaseImpl::onDecompressError);
  dec.decompressAllFiles();
  dec.EError -= Poco::Delegate<
      DatabaseImpl,
      std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>>(
      this, &DatabaseImpl::onDecompressError);

  if (debug) {
    LOG(INFO) << "Restore: Create backup engine";
    LOG(INFO) << "Restore from" << backupPath;
  }

  BackupEngineReadOnly *backup_engine;
  rocksdb::Status s = BackupEngineReadOnly::Open(
      Env::Default(), BackupableDBOptions(backupPath), &backup_engine);
  if (!s.ok()) {
    LOG(ERROR) << "Error:" << s.ToString();
  }
  BackupInfo info;
  s = backup_engine->GetLatestBackupInfo(&info, false);
  if (s.IsNotFound()) {
    LOG(ERROR) << "No backup found, so nothing to restore";
  } else {
    s = backup_engine->RestoreDBFromLatestBackup(databaseName, databaseName);
    if (!s.ok()) {
      LOG(ERROR) << "Error: " << s.ToString();
      return grpc::Status::CANCELLED;
    }
  }
  delete backup_engine;

  return grpc::Status::OK;
}