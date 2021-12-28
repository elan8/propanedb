#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "propanedb.grpc.pb.h"

#include "test.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

//propane::Database::NewStub
class Client
{
public:
  Client(std::shared_ptr<Channel> channel);

  grpc::Status CreateDatabase();
  grpc::Status Backup();
  grpc::Status Restore();
  grpc::Status Put( test::TestEntity entity, std::string *id);
  grpc::Status Get( std::string id, test::TestEntity *entity);
  grpc::Status Delete( std::string id);

private:
  std::unique_ptr<propane::Database::Stub> stub_;
  std::string databaseName;
};
