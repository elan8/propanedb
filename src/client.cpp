/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <streambuf>

#include <grpcpp/grpcpp.h>
#include "propanedb.grpc.pb.h"
#include "test.pb.h"

using google::protobuf::Any;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using propane::Database;
using propane::PropaneEntity;
using propane::PropaneFileDescriptor;
using propane::PropaneId;
using propane::PropaneStatus;
using test::TestEntity;

using namespace std;

class DatabaseClient
{
public:
  DatabaseClient(std::shared_ptr<Channel> channel)
      : stub_(Database::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string Put()
  {

    TestEntity entity;
    string id("abc");
    string data("Test");
    entity.set_id(id);
    entity.set_data(data);

    Any *anyMessage = new Any();
    anyMessage->PackFrom(entity);

    // Data we are sending to the server.
    PropaneEntity request;
    request.set_allocated_data(anyMessage);

    // Container for the data we expect from the server.
    PropaneId reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->Put(&context, request, &reply);

    // Act upon its status.
    if (status.ok())
    {
      return reply.id();
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  google::protobuf::Any Get(string id)
  {
    PropaneId request;
    request.set_id(id);
    PropaneEntity reply;
    ClientContext context;
    Status status = stub_->Get(&context, request, &reply);
    if (status.ok())
    {
      return reply.data();
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      //return "RPC failed";
      google::protobuf::Any any;
      return any;
    }
  }

  std::string SetFileDescriptor(propane::PropaneFileDescriptor request)
  {

    PropaneStatus reply;
    ClientContext context;
    Status status = stub_->SetFileDescriptor(&context, request, &reply);

    // PropaneId request;
    // request.set_id(id);
    // PropaneEntity reply;

    // Status status = stub_->Get(&context, request, &reply);
    if (status.ok())
    {
      return reply.statusmessage();
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
      //google::protobuf::Any any;
      //return any;
    }
  }

private:
  std::unique_ptr<Database::Stub> stub_;
};

int main(int argc, char **argv)
{
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  std::string target_str;
  std::string arg_str("--target");
  if (argc > 1)
  {
    std::string arg_val = argv[1];
    size_t start_pos = arg_val.find(arg_str);
    if (start_pos != std::string::npos)
    {
      start_pos += arg_str.size();
      if (arg_val[start_pos] == '=')
      {
        target_str = arg_val.substr(start_pos + 1);
      }
      else
      {
        std::cout << "The only correct argument syntax is --target="
                  << std::endl;
        return 0;
      }
    }
    else
    {
      std::cout << "The only acceptable argument is --target=" << std::endl;
      return 0;
    }
  }
  else
  {
    target_str = "localhost:50051";
  }
  DatabaseClient propaneClient(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  std::ifstream t("descriptor.bin");
  std::string descriptor((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());

  google::protobuf::FileDescriptorSet *fd = new google::protobuf::FileDescriptorSet;
  fd->ParseFromString(descriptor);
  std::cout << "Descriptor: " << fd->DebugString() << std::endl;

  propane::PropaneFileDescriptor pfd;
  pfd.set_allocated_descriptor_set(fd);
  std::string reply =propaneClient.SetFileDescriptor(pfd);
  std::cout << "SetFileDescriptor: " << reply << std::endl;

  std::string id = propaneClient.Put();
  std::cout << "Put: ID received: " << id << std::endl;

  google::protobuf::Any any = propaneClient.Get(id);
  std::cout << "Get: any receive: " << any.DebugString() << std::endl;

  return 0;
}