#include <filesystem>
#include <cstdio>
namespace fs = std::filesystem;

#include "DatabaseServiceImpl.hpp"
#include "BackupReader.hpp"
#include "FileWriter.hpp"

DatabaseServiceImpl::DatabaseServiceImpl(const string &databasePath, const string &backupPath, bool debug) : databasePath(databasePath), backupPath(backupPath)
{

  implementation = new DatabaseImpl(databasePath, backupPath, debug);
  this->debug = debug;
}

DatabaseServiceImpl::~DatabaseServiceImpl()
{
  delete implementation;
}

Metadata DatabaseServiceImpl::GetMetadata(ServerContext *context)
{
  Metadata metadata;
  auto map = context->client_metadata();
  auto search = map.find("database-name");
  if (search != map.end())
  {
    const char *data = (search->second).data();
    //LOG(INFO) << "length= " << (search->second).length();
    metadata.databaseName = std::string(data, (search->second).length());
    if (debug)
    {
      LOG(INFO) << "databaseName :" << metadata.databaseName;
    }
  }
  return metadata;
}

grpc::Status DatabaseServiceImpl::CreateDatabase(ServerContext *context, const propane::PropaneDatabaseRequest *request,
                                                 propane::PropaneStatus *reply)
{
  Metadata meta = this->GetMetadata(context);
  return implementation->CreateDatabase(&meta, request, reply);
}

grpc::Status DatabaseServiceImpl::UpdateDatabase(ServerContext *context, const propane::PropaneDatabaseRequest *request,
                                                 propane::PropaneStatus *reply)
{
  Metadata meta = this->GetMetadata(context);
  return implementation->UpdateDatabase(&meta, request, reply);
}

grpc::Status DatabaseServiceImpl::DeleteDatabase(ServerContext *context, const propane::PropaneDatabaseRequest *request,
                                                 propane::PropaneStatus *reply)
{
  Metadata meta = this->GetMetadata(context);
  return implementation->DeleteDatabase(&meta, request, reply);
}

grpc::Status DatabaseServiceImpl::Put(ServerContext *context, const propane::PropanePut *request,
                                      propane::PropaneId *reply)
{
  Metadata meta = this->GetMetadata(context);
  return implementation->Put(&meta, request, reply);
}

grpc::Status DatabaseServiceImpl::Get(ServerContext *context, const propane::PropaneId *request,
                                      propane::PropaneEntity *reply)
{
  Metadata meta = this->GetMetadata(context);
  return implementation->Get(&meta, request, reply);
}

grpc::Status DatabaseServiceImpl::Delete(ServerContext *context, const propane::PropaneId *request,
                                         propane::PropaneStatus *reply)
{
  Metadata meta = this->GetMetadata(context);
  return implementation->Delete(&meta, request, reply);
}

grpc::Status DatabaseServiceImpl::Search(ServerContext *context, const propane::PropaneSearch *request,
                                         propane::PropaneEntities *reply)
{
  Metadata meta = this->GetMetadata(context);
  return implementation->Search(&meta, request, reply);
}

grpc::Status DatabaseServiceImpl::Backup(ServerContext *context, const ::propane::PropaneBackupRequest *request,
                                         ::grpc::ServerWriter<::propane::PropaneBackupReply> *writer)
{
  Metadata meta = this->GetMetadata(context);
  string databaseName = request->databasename();
  string zipFilePath = "/tmp/backup.zip";
  implementation->Backup(&meta, databaseName, zipFilePath);
  BackupReader reader(zipFilePath, writer);

  const size_t chunk_size = 1UL << 20; // Hardcoded to 1MB, which seems to be recommended from experience.
  reader.Read(chunk_size);

  if (remove(zipFilePath.c_str()) != 0)
  {
    LOG(ERROR) << "Error deleting file" << endl;
  }

  return grpc::Status::OK;
}
grpc::Status DatabaseServiceImpl::Restore(ServerContext *context, ::grpc::ServerReader<::propane::PropaneRestoreRequest> *reader,
                                          ::propane::PropaneRestoreReply *response)
{
  FileWriter writer;

  Metadata meta = this->GetMetadata(context);


  string databaseName = "restore";
  string zipFilePath = "/tmp/" + databaseName + ".zip";

  if (debug)
  {
    LOG(INFO) << "Restore: zipFilePath = " << zipFilePath << endl;
  }

  writer.OpenIfNecessary(zipFilePath);
  propane::PropaneRestoreRequest contentPart;
  reader->SendInitialMetadata();
  while (reader->Read(&contentPart))
  {
    databaseName = contentPart.chunk().databasename();
    auto d = contentPart.chunk().data();
    writer.Write(d);
  }

  if (debug)
  {
    LOG(INFO) << "Restore: databaseName = " << databaseName << endl;
  }

  implementation->Restore(&meta, databasePath + "/" + databaseName, zipFilePath);

  return grpc::Status::OK;
}