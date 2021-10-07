#include <filesystem>
namespace fs = std::filesystem;

#include "DatabaseServiceImpl.h"

DatabaseServiceImpl::DatabaseServiceImpl(string path)
{
  directory = path;
  descriptorDB = new google::protobuf::SimpleDescriptorDatabase();
  pool = new google::protobuf::DescriptorPool(descriptorDB);
  queryParser = new QueryParser();
}

DatabaseServiceImpl::~DatabaseServiceImpl()
{
  for (auto it = databases.begin(); it != databases.end(); ++it)
  {
    DB *db = it->second;
    db->Close();
  }

  delete descriptorDB;
}

grpc::Status DatabaseServiceImpl::CreateDatabase(ServerContext *context, const propane::PropaneDatabase *request,
                                                 propane::PropaneStatus *reply)
{
  //LOG(INFO) << "CreateDatabase" << endl;
  DB *db;
  string name = request->databasename();
   LOG(INFO) << "CreateDatabase databaseName="<<name << endl;
  fs::path p1;
  p1 += directory;
  p1 /= name;
  string path = p1.generic_string();
  LOG(INFO) << "path="<< path << endl;

  google::protobuf::FileDescriptorSet fds = request->descriptor_set();
  descriptorDB = new google::protobuf::SimpleDescriptorDatabase();

  auto file = fds.file();
  for (auto it = file.begin(); it != file.end(); ++it)
  {
    descriptorDB->Add((*it));
  }
  pool = new google::protobuf::DescriptorPool(descriptorDB);

  Options options;
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  options.create_if_missing = true;
  // create and open DB
  ROCKSDB_NAMESPACE::Status s = DB::Open(options, path, &db);
  LOG(INFO) << "Status code="s << s.ToString() << endl;
  assert(s.ok());

  databases[name] = db;

  return grpc::Status::OK;
}

rocksdb::DB *DatabaseServiceImpl::GetDatabase(string name)
{
  LOG(INFO) << "GetDatabase" << endl;
  fs::path p1;
  p1 += directory;
  p1 /= name;
  string path = p1.generic_string();
  LOG(INFO) << "Database name=" << name << " path=" << path << endl;

  if (name.length() == 0)
  {
    LOG(ERROR) << "Error: Database name=empty" << endl;
    return 0;
  }

  DB *db = databases[name];
  if (db == nullptr)
  {
    LOG(INFO) << "Database pointer = null: opening database" << endl;
    Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    //options.create_if_missing = true;
    // open DB
    ROCKSDB_NAMESPACE::Status s = DB::Open(options, path, &db);
    LOG(INFO) << "Status code="s << s.ToString() << endl;
    

    assert(s.ok());
    databases[name] = db;
  }

  return db;
}

grpc::Status DatabaseServiceImpl::Put(ServerContext *context, const propane::PropanePut *request,
                                      propane::PropaneId *reply)
{
  LOG(INFO) << "Put" << endl;
  string name = request->databasename();

  Any any = (request->entity()).data();
  string typeUrl = any.type_url();
  LOG(INFO) << "Any TypeURL=" << typeUrl << endl;
  string typeName = Util::getTypeName(typeUrl);
  LOG(INFO) << "Any TypeName=" << typeName << endl;
  // cout << "Descriptor pool=" << descriptorDB-> << endl;
  const google::protobuf::Descriptor *descriptor = pool->FindMessageTypeByName(typeName);
  if (descriptor != nullptr)
  {
    //LOG(INFO) << "Descriptor=" << descriptor->DebugString() << endl;
    google::protobuf::Message *message = dmf.GetPrototype(descriptor)->New();
    any.UnpackTo(message);
    //LOG(INFO) << "Message INFO String=" << message->DebugString() << endl;

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

    ROCKSDB_NAMESPACE::Status s = GetDatabase(name)->Put(WriteOptions(), id, serializedAny);
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
  LOG(INFO) << "Get" << endl;
  string name = request->databasename();

  string serializedAny;
  ROCKSDB_NAMESPACE::Status s = GetDatabase(name)->Get(ReadOptions(), request->id(), &serializedAny);
  google::protobuf::Any *any = new Any();
  any->ParseFromString(serializedAny);
  reply->set_allocated_data(any);
  return grpc::Status::OK;
}

grpc::Status DatabaseServiceImpl::Delete(ServerContext *context, const propane::PropaneId *request,
                                         propane::PropaneStatus *reply)
{
  string name = request->databasename();
  string serializedAny;
  ROCKSDB_NAMESPACE::Status s = GetDatabase(name)->Get(ReadOptions(), request->id(), &serializedAny);
  if (s.ok())
  {
    ROCKSDB_NAMESPACE::Status s = GetDatabase(name)->Delete(WriteOptions(), request->id());
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
  LOG(INFO) << "Search" << endl;
  string name = request->databasename();
  ROCKSDB_NAMESPACE::Iterator *it = GetDatabase(name)->NewIterator(ReadOptions());

  Query query = queryParser->parseQuery(request->query());
  if (query.hasError())
  {
    LOG(ERROR) << "Query error: =: " << query.getErrorMessage() << std::endl;
  }

  reply->clear_entities();

  google::protobuf::RepeatedPtrField<::propane::PropaneEntity> *entities = reply->mutable_entities();

  //LOG(ERROR) << "Entity =: " << entity->DebugString() << std::endl;

  //LOG(ERROR) << "Entities length =: " << reply->entities_size() << std::endl;

  for (it->Seek(""); it->Valid(); it->Next())
  {
    //LOG(INFO) << "Search: key=: " << it->key().ToString() << std::endl;
    google::protobuf::Any *any = new Any();
    any->ParseFromString(it->value().ToString());
    string typeUrl = any->type_url();
    //LOG(INFO) << "Any TypeURL=" << typeUrl << endl;
    string typeName = Util::getTypeName(typeUrl);
    //LOG(INFO) << "Any TypeName=" << typeName << endl;
    // cout << "Descriptor pool=" << descriptorDB-> << endl;
    if (IsCorrectEntityType(any, request->entitytype()))
    {
      const google::protobuf::Descriptor *descriptor = pool->FindMessageTypeByName(typeName);
      if (descriptor != nullptr)
      {

        //LOG(INFO) << "Unpack to message " << endl;
        google::protobuf::Message *message = dmf.GetPrototype(descriptor)->New();
        any->UnpackTo(message);
        //LOG(INFO) << "Message INFO String=" << message->DebugString() << endl;

        if (query.isMatch(descriptor, message))
        {
          propane::PropaneEntity *entity = entities->Add();
          entity->set_allocated_data(any);
          LOG(INFO) << "Search:: Add entity" << endl;
        }
      }
    }
  }
  return grpc::Status::OK;
}

bool DatabaseServiceImpl::IsCorrectEntityType(google::protobuf::Any *any, std::string entityType)
{
  LOG(INFO) << "IsCorrectEntityType" << endl;
  bool output = false;
  string typeUrl = any->type_url();
  string typeName = Util::getTypeName(typeUrl);
  if (entityType.compare(typeName) == 0)
  {
    output = true;
  }
  return output;
}

// string DatabaseServiceImpl::GetDatabaseNameFromContext(grpc::ServerContext* context){
//   string name = "";
//   auto map = context->client_metadata();
//   auto search = map.find("database");

//   if (search != map.end())
//   {
//     name = search->second.data() ;
//   }

//   return name;
// }