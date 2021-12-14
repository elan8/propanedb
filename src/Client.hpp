#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "propanedb.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;


//propane::Database::NewStub
class Client {
 public:
  Client(std::shared_ptr<Channel> channel);

  grpc::Status  CreateDatabase(std::string databaseName);
  grpc::Status  Backup(std::string databaseName);
  
 private:
  std::unique_ptr<propane::Database::Stub> stub_;
};
