
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <glog/logging.h>
#include <grpcpp/grpcpp.h>
#include "propanedb.grpc.pb.h"
#include "test.pb.h"
#include "Client.hpp"
#include "FileWriter.hpp"
#include "RestoreReader.hpp"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <google/protobuf/dynamic_message.h>
//#include <google/protobuf/any.h>

using google::protobuf::Any;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

Client::Client(std::shared_ptr<Channel> channel)
    : stub_(propane::Database::NewStub(channel)), databaseName("test")
{
}

grpc::Status Client::CreateDatabase()
{
    LOG(INFO) << "Client: CreateDatabase" << std::endl;
    ClientContext context;

    std::ifstream fileStream("descriptor.bin");
    std::string descriptor((std::istreambuf_iterator<char>(fileStream)),
                           std::istreambuf_iterator<char>());
    google::protobuf::FileDescriptorSet *fd = new google::protobuf::FileDescriptorSet;
    fd->ParseFromString(descriptor);
    //    LOG(INFO) << "Client: fd =" <<  fd->DebugString() << std::endl;

    propane::PropaneDatabase request;
    request.set_databasename(databaseName);
    request.set_allocated_descriptor_set(fd);

    propane::PropaneStatus response;
    return stub_->CreateDatabase(&context, request, &response);
}

grpc::Status Client::Put(test::TestEntity item, std::string *id)
{
    ClientContext context;
    context.AddMetadata("database-name", databaseName);
    propane::PropaneId response;
    propane::PropanePut request;
    propane::PropaneEntity *entity = new propane::PropaneEntity();
    Any *anyMessage = new Any();
    anyMessage->PackFrom(item);
    entity->set_allocated_data(anyMessage);
    request.set_allocated_entity(entity);
    //request.set_databasename("test");

    auto status = stub_->Put(&context, request, &response);

    *id = response.id();
    LOG(INFO) << "Put: ID= " << response.id() << std::endl;

    //output.copy(id,output.length());
    //id = *(&output);

    return status;
}

grpc::Status Client::Get(std::string id, test::TestEntity *entity)
{
    ClientContext context;
    context.AddMetadata("database-name", "test");
    propane::PropaneId request;
    request.set_id(id);

    propane::PropaneEntity response;
    auto status = stub_->Get(&context, request, &response);

    LOG(INFO) << "Get: response data: " << response.data().DebugString() << std::endl;

    auto any = response.data();
    any.UnpackTo(entity);

    return status;
}

grpc::Status Client::Delete(std::string id)
{
    LOG(INFO) << "Delete" << std::endl;
    ClientContext context;
    context.AddMetadata("database-name", "test");

    propane::PropaneId request;
    propane::PropaneStatus response;
    request.set_id(id);

    return stub_->Delete(&context, request, &response);
}

grpc::Status Client::Backup()
{
    ClientContext context;

    propane::PropaneBackupRequest request;
    request.set_databasename(databaseName);
    propane::PropaneBackupReply *reply = new propane::PropaneBackupReply();

    auto streamReader = stub_->Backup(&context, request);
    auto reader = streamReader.get();
    FileWriter writer;
    writer.OpenIfNecessary("backup.zip");

    while (reader->Read(reply))
    {
        LOG(INFO) << "Client: Backup data received" << std::endl;
        std::string data = reply->chunk().data();
        writer.Write(data);
    }
    LOG(INFO) << "Client: Backup data complete" << std::endl;

    auto status = reader->Finish();

    LOG(INFO) << "Backup: reader finish: " << status.error_details() << std::endl;
    return grpc::Status::OK;
}

grpc::Status Client::Restore()
{
    ClientContext context;
    propane::PropaneRestoreReply response;

    const size_t chunk_size = 1UL << 20; // Hardcoded to 1MB, which seems to be recommended from experience.

    auto writerPointer = stub_->Restore(&context, &response);
    auto writer = writerPointer.get();
    RestoreReader reader("backup.zip", writer);
    writer->WaitForInitialMetadata();

    reader.Read(chunk_size);

    return grpc::Status::OK;
}