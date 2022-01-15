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
//template <class myType>
class Client
{
public:
  Client(std::shared_ptr<Channel> channel);

  grpc::Status CreateDatabase();
  grpc::Status Backup();
  grpc::Status Restore();


  grpc::Status Put( google::protobuf::Message *entity, std::string *id);
  grpc::Status Get( std::string id, google::protobuf::Message *entity);
  grpc::Status Delete( std::string id);
  grpc::Status Search( std::string entityType,std::string query,propane::PropaneEntities *entities);

private:
  std::unique_ptr<propane::Database::Stub> stub_;
  std::string databaseName;
};
