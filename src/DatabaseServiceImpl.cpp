#include "DatabaseServiceImpl.h"

DatabaseServiceImpl::DatabaseServiceImpl(string path)
{

  //google::InitGoogleLogging("");
  Options options;
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  // create the DB if it's not already present
  options.create_if_missing = true;
  // open DB
  ROCKSDB_NAMESPACE::Status s = DB::Open(options, path, &db);
  assert(s.ok());

  descriptorDB = new google::protobuf::SimpleDescriptorDatabase();
  pool = new google::protobuf::DescriptorPool(descriptorDB);
  queryParser = new QueryParser();
}

DatabaseServiceImpl::~DatabaseServiceImpl()
{
  db->Close();
  delete descriptorDB;
  delete db;
}

grpc::Status DatabaseServiceImpl::Put(ServerContext *context, const propane::PropaneEntity *request,
                                      propane::PropaneId *reply)
{
  Any any = request->data();
  string typeUrl = any.type_url();
  LOG(INFO) << "Any TypeURL=" << typeUrl << endl;
  string typeName = Util::getTypeName(typeUrl);
  LOG(INFO) << "Any TypeName=" << typeName << endl;
  // cout << "Descriptor pool=" << descriptorDB-> << endl;
  const google::protobuf::Descriptor *descriptor = pool->FindMessageTypeByName(typeName);
  if (descriptor != nullptr)
  {
    LOG(INFO) << "Descriptor=" << descriptor->DebugString() << endl;
    google::protobuf::Message *message = dmf.GetPrototype(descriptor)->New();
    any.UnpackTo(message);
    LOG(INFO) << "Message INFO String=" << message->DebugString() << endl;

    const google::protobuf::FieldDescriptor *fd = descriptor->FindFieldByName("id");
    const google::protobuf::Reflection *reflection = message->GetReflection();
    string id = reflection->GetString(*message, fd);
    if (id.length() == 0)
    {
      id = Util::generateUUID();
      reflection->SetString(message, fd, id);
    }
    LOG(INFO) << "Message ID= " << id << endl;
    string serializedAny;
    any.SerializeToString(&serializedAny);

    ROCKSDB_NAMESPACE::Status s = db->Put(WriteOptions(), id, serializedAny);
    assert(s.ok());
    reply->set_id(id);
  }
  else
  {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "Descriptor not found");
  }
  return grpc::Status::OK;
}

grpc::Status DatabaseServiceImpl::Get(ServerContext *context, const propane::PropaneId *request,
                                      propane::PropaneEntity *reply)
{
  string serializedAny;
  ROCKSDB_NAMESPACE::Status s = db->Get(ReadOptions(), request->id(), &serializedAny);
  google::protobuf::Any *any = new Any();
  any->ParseFromString(serializedAny);
  reply->set_allocated_data(any);
  return grpc::Status::OK;
}

grpc::Status DatabaseServiceImpl::Delete(ServerContext *context, const propane::PropaneId *request,
                                         propane::PropaneStatus *reply)
{
  //throw an error if no object with this key exists
  string serializedAny;
  ROCKSDB_NAMESPACE::Status s = db->Get(ReadOptions(), request->id(), &serializedAny);
  if (s.ok())
  {
    ROCKSDB_NAMESPACE::Status s = db->Delete(WriteOptions(), request->id());
    LOG(INFO) << "Delete status= " << s.ToString() << endl;
    if (!s.ok())
    {
      return grpc::Status(grpc::StatusCode::ABORTED, "Could not delete object");
      //status = new grpc::Status::Status(grpc::StatusCode::ABORTED, "Could not delete object");
    }
  }
  else
  {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "No object with this ID in database");
  }
  return grpc::Status::OK;
}

grpc::Status DatabaseServiceImpl::Search(ServerContext *context, const propane::PropaneSearch *request,
                                         propane::PropaneEntities *reply)
{
  ROCKSDB_NAMESPACE::Iterator *it = db->NewIterator(ReadOptions());

  Query query = queryParser->parseQuery(request->query());
  if (query.hasError())
  {
    LOG(ERROR) << "Query error: =: " << query.getErrorMessage() << std::endl;
  }

  propane::PropaneEntity *entity = reply->add_entities();

  for (it->Seek(""); it->Valid(); it->Next())
  {
    LOG(INFO) << "Search: key=: " << it->key().ToString() << std::endl;
    google::protobuf::Any *any = new Any();
    any->ParseFromString(it->value().ToString());
    string typeUrl = any->type_url();
    LOG(INFO) << "Any TypeURL=" << typeUrl << endl;
    string typeName = Util::getTypeName(typeUrl);
    LOG(INFO) << "Any TypeName=" << typeName << endl;
    // cout << "Descriptor pool=" << descriptorDB-> << endl;
    if (IsCorrectEntityType(any, request->entitytype()))
    {
      const google::protobuf::Descriptor *descriptor = pool->FindMessageTypeByName(typeName);
      if (descriptor != nullptr)
      {

        LOG(INFO) << "Unpack to message " << endl;
        google::protobuf::Message *message = dmf.GetPrototype(descriptor)->New();
        any->UnpackTo(message);
        LOG(INFO) << "Message INFO String=" << message->DebugString() << endl;

        if (query.isMatch(message))
        {
          entity->set_allocated_data(any);
          cout << "Search:: Add entity" << endl;
        }
      }
    }
  }
  return grpc::Status::OK;
}

grpc::Status DatabaseServiceImpl::SetFileDescriptor(ServerContext *context, const propane::PropaneFileDescriptor *request,
                                                    propane::PropaneStatus *reply)
{
  google::protobuf::FileDescriptorSet fds = request->descriptor_set();
  descriptorDB = new google::protobuf::SimpleDescriptorDatabase();

  auto file = fds.file();
  for (auto it = file.begin(); it != file.end(); ++it)
  {
    descriptorDB->Add((*it));
  }
  pool = new google::protobuf::DescriptorPool(descriptorDB);
  return grpc::Status::OK;
}

bool DatabaseServiceImpl::IsCorrectEntityType(google::protobuf::Any *any, std::string entityType)
{
  bool output = false;
  string typeUrl = any->type_url();
  //LOG(INFO) << "Any TypeURL=" << typeUrl << endl;
  string typeName = Util::getTypeName(typeUrl);
  //LOG(INFO) << "Any TypeName=" << typeName << endl;
  if (entityType.compare(typeName) == 0)
  {
    output = true;
  }
  return output;
}