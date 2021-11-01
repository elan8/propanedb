#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

#include <glog/logging.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <google/protobuf/dynamic_message.h>
#include "DatabaseServiceImpl.hpp"

using google::protobuf::Any;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using namespace std;

std::string kDBPath = "/var/rocksdb";
bool debug = false;

void RunServer()
{
  std::string server_address("0.0.0.0:50051");
  DatabaseServiceImpl service(kDBPath, debug);
  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  LOG(INFO) << "PropaneDB started: Listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char **argv)
{
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  char* flag = std::getenv("DEBUG");
  if (flag)
  {
    if (std::string("true").compare(flag) == 0)
    {
      LOG(INFO) << "DEBUG MODE ENABLED" << '\n';
      debug = true;
    }
    else
    {
      debug = false;
    }
  }

  RunServer();
  return 0;
}