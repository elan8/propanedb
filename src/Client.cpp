
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <glog/logging.h>
#include <grpcpp/grpcpp.h>
#include "propanedb.grpc.pb.h"
#include "Client.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

Client::Client(std::shared_ptr<Channel> channel)
    : stub_(propane::Database::NewStub(channel))
{
}

grpc::Status Client::CreateDatabase(std::string databaseName)
{
    LOG(INFO) << "Client: CreateDatabase" << std::endl;
    ClientContext context;

    std::ifstream fileStream("descriptor.bin");
    std::string descriptor((std::istreambuf_iterator<char>(fileStream)),
                           std::istreambuf_iterator<char>());
    google::protobuf::FileDescriptorSet *fd = new google::protobuf::FileDescriptorSet;
    fd->ParseFromString(descriptor);

    propane::PropaneDatabase request;
    request.set_databasename(databaseName);
    request.set_allocated_descriptor_set(fd);

    propane::PropaneStatus response;
    return stub_->CreateDatabase(&context, request, &response);
}

grpc::Status Client::Backup(std::string databaseName)
{
    ClientContext context;

    propane::PropaneBackupRequest request;
    request.set_databasename(databaseName);
    propane::PropaneBackupReply *reply = new propane::PropaneBackupReply();

    auto streamReader = stub_->Backup(&context, request);
    auto reader = streamReader.get();

    while (reader->Read(reply))
    {
        LOG(INFO) << "Client: Backup data received" << std::endl;
    }
    LOG(INFO) << "Client: Backup data complete" << std::endl;

    return reader->Finish();
}

// Assembles the client's payload, sends it and presents the response back
// from the server.
//   std::string SayHello(const std::string& user) {
//     // Data we are sending to the server.
//     HelloRequest request;
//     request.set_name(user);

//     // Container for the data we expect from the server.
//     HelloReply reply;

//     // Context for the client. It could be used to convey extra information to
//     // the server and/or tweak certain RPC behaviors.
//     ClientContext context;

//     // The actual RPC.
//     Status status = stub_->SayHello(&context, request, &reply);

//     // Act upon its status.
//     if (status.ok()) {
//       return reply.message();
//     } else {
//       std::cout << status.error_code() << ": " << status.error_message()
//                 << std::endl;
//       return "RPC failed";
//     }
//   }

//};
