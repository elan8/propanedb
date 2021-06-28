#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <google/protobuf/dynamic_message.h>
#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
#include "propanedb.grpc.pb.h"
#include "util.h"

using google::protobuf::Any;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using propane::Database;
using propane::PropaneEntity;
using propane::PropaneId;
using propane::PropaneStatus;
using propane::PropaneFileDescriptor;

using namespace ROCKSDB_NAMESPACE;
using namespace std;

#if defined(OS_WIN)
std::string kDBPath = "C:\\Windows\\TEMP\\rocksdb_simple_example";
#else
std::string kDBPath = "/tmp/rocksdb_simple_example";
#endif

class DatabaseServiceImpl final : public Database::Service
{
  private:
  DB *db;

public:
  DatabaseServiceImpl()
  {

    Options options;
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    // create the DB if it's not already present
    options.create_if_missing = true;
    // open DB
    ROCKSDB_NAMESPACE::Status s = DB::Open(options, kDBPath, &db);
    assert(s.ok());
  }

  ~DatabaseServiceImpl()
  {

    db->Close();
    delete db;
  }



  grpc::Status Put(ServerContext *context, const PropaneEntity *request,
                   PropaneId *reply) override
  {
    google::protobuf::DynamicMessageFactory dmf;
    const google::protobuf::DescriptorPool *pool = google::protobuf::DescriptorPool::generated_pool();


    Any any = request->data();
    string typeUrl = any.type_url();
    cout << "Any TypeURL=" << typeUrl << endl;
    string typeName = Util::split(typeUrl, "/");
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
    else{
       cout << "Descriptor not found"  << endl;
    }

    
    return grpc::Status::OK;
  }

  grpc::Status Get(ServerContext *context, const PropaneId *request,
                   PropaneEntity *reply) override
  {

    string serializedAny;
    ROCKSDB_NAMESPACE::Status s = db->Get(ReadOptions(), request->id(), &serializedAny);
    
    google::protobuf::Any* any = new Any();
    any->ParseFromString(serializedAny);

    reply->set_allocated_data(any);
    //reply->set_statusmessage("OK");
    return grpc::Status::OK;
  }

    grpc::Status SetFileDescriptor(ServerContext *context, const PropaneFileDescriptor *request,
                   PropaneStatus*reply) override
  {

    // string serializedAny;
    // ROCKSDB_NAMESPACE::Status s = db->Get(ReadOptions(), request->id(), &serializedAny);
    
    // google::protobuf::Any* any = new Any();
    // any->ParseFromString(serializedAny);

    // reply->set_allocated_data(any);
    // //reply->set_statusmessage("OK");
    return grpc::Status::OK;
  }


};

void RunServer()
{
  std::string server_address("0.0.0.0:50051");

  DatabaseServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char **argv)
{
  RunServer();

  return 0;
}