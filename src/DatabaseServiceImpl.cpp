#include "DatabaseServiceImpl.h"


DatabaseServiceImpl::DatabaseServiceImpl(string path)
  {

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
  }

  DatabaseServiceImpl::~DatabaseServiceImpl()
  {

    db->Close();
    delete descriptorDB;
    delete db;
  }

  grpc::Status DatabaseServiceImpl::Put(ServerContext *context, const PropaneEntity *request,
                   PropaneId *reply)
  {
    google::protobuf::DynamicMessageFactory dmf;
    const google::protobuf::DescriptorPool *pool = new google::protobuf::DescriptorPool(descriptorDB);

    Any any = request->data();
    string typeUrl = any.type_url();
    cout << "Any TypeURL=" << typeUrl << endl;
    string typeName = Util::getTypeName(typeUrl);
    cout << "Any TypeName=" << typeName << endl;
    const google::protobuf::Descriptor *descriptor = pool->FindMessageTypeByName(typeName);
    if (descriptor != nullptr)
    {
      cout << "Descriptor=" << descriptor->DebugString() << endl;
      google::protobuf::Message *message = dmf.GetPrototype(descriptor)->New();
      any.UnpackTo(message);
      cout << "Message Debug String=" << message->DebugString() << endl;

      const google::protobuf::FieldDescriptor *fd = descriptor->FindFieldByName("id");
      const google::protobuf::Reflection *reflection = message->GetReflection();
      string id = reflection->GetString(*message, fd);
      cout << "Message ID= " << id << endl;

      string serializedAny;
      any.SerializeToString(&serializedAny);

      ROCKSDB_NAMESPACE::Status s = db->Put(WriteOptions(), id, serializedAny);
      assert(s.ok());
      reply->set_id(id);
    }
    else
    {
      cout << "Descriptor not found" << endl;
    }

    return grpc::Status::OK;
  }

  grpc::Status DatabaseServiceImpl::Get(ServerContext *context, const PropaneId *request,
                   PropaneEntity *reply) 
  {
    string serializedAny;
    ROCKSDB_NAMESPACE::Status s = db->Get(ReadOptions(), request->id(), &serializedAny);
    google::protobuf::Any *any = new Any();
    any->ParseFromString(serializedAny);
    reply->set_allocated_data(any);
    return grpc::Status::OK;
  }

  grpc::Status DatabaseServiceImpl::SetFileDescriptor(ServerContext *context, const PropaneFileDescriptor *request,
                                 PropaneStatus *reply) 
  {
    google::protobuf::FileDescriptorSet fds = request->descriptor_set();
    auto file = fds.file();
    for (auto it = file.begin(); it != file.end(); ++it) {
      descriptorDB->Add( (*it) );
    }
    return grpc::Status::OK;
  }
