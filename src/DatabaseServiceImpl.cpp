#include <filesystem>
namespace fs = std::filesystem;

#include "DatabaseServiceImpl.hpp"
#include "FileReader.hpp"
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

  std::multimap<grpc::string_ref, grpc::string_ref> data = context->client_metadata();
  if (debug)
  {
    LOG(INFO) << "GetMetadata from context:" << endl;
    LOG(INFO) << data.size();
    for (auto iter = data.begin(); iter != data.end(); ++iter)
    {
      LOG(INFO) << "Header key: " << iter->first << ", value: " << iter->second;
    }
  }

  auto map = context->client_metadata();
  auto search = map.find("database-name");
  if (search != map.end())
  {
    metadata.databaseName = (search->second).data();
    if (debug)
    {
      LOG(INFO) << "databaseName :" << metadata.databaseName;
    }
  }
  return metadata;
}

grpc::Status DatabaseServiceImpl::CreateDatabase(ServerContext *context, const propane::PropaneDatabase *request,
                                                 propane::PropaneStatus *reply)
{
  Metadata meta = this->GetMetadata(context);
  return implementation->CreateDatabase(&meta, request, reply);
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
  string zipFilePath = databasePath + "/" + databaseName + ".zip";
  implementation->Backup(&meta, databaseName, zipFilePath);

  //TODO: upload contents of the ZIP file as chunks of bytes in a stream
  FileReader reader(zipFilePath, writer);

  const size_t chunk_size = 1UL << 20; // Hardcoded to 1MB, which seems to be recommended from experience.
  reader.Read(chunk_size);

  return grpc::Status::OK;
}
grpc::Status DatabaseServiceImpl::Restore(ServerContext *context, ::grpc::ServerReader<::propane::PropaneRestoreRequest> *reader,
                                          ::propane::PropaneRestoreReply *response)
{

  FileWriter writer;

  //  propane::PropaneRestoreRequest contentPart;
  //       SequentialFileWriter writer;
  //       while (reader->Read(&contentPart)) {
  //           try {
  //               // FIXME: Do something reasonable if a file with a different name but the same ID already exists
  //               writer.OpenIfNecessary(contentPart.name());
  //               auto* const data = contentPart.mutable_content();
  //               writer.Write(*data);

  //               //summary->set_id(contentPart.id());
  //               // FIXME: Protect from concurrent access by multiple threads
  //               //m_FileIdToName[contentPart.id()] = contentPart.name();
  //           }
  //           catch (const std::system_error& ex) {
  //               const auto status_code = writer.NoSpaceLeft() ? grpc::StatusCode::RESOURCE_EXHAUSTED : grpc::StatusCode::ABORTED;
  //               return grpc::Status(status_code, ex.what());
  //           }
  //       }

  //TODO: Assemble ZIP file from chunks of data
  Metadata meta = this->GetMetadata(context);
  string databaseName = "";
  string zipFilePath = databasePath + "/" + databaseName + ".zip";

  writer.OpenIfNecessary(zipFilePath);

  propane::PropaneRestoreRequest contentPart;
  //       SequentialFileWriter writer;
  while (reader->Read(&contentPart))
  {
   auto d = contentPart.chunk().data();
    writer.Write(d);
  }

  implementation->Restore(&meta, databasePath, zipFilePath);

  return grpc::Status::OK;
}