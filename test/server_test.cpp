#include <iostream>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <streambuf>
#include <gtest/gtest.h>
#include "test.pb.h"
#include "DatabaseServiceImpl.h"
#include <boost/filesystem.hpp>

using namespace std;

class PropanedbTest : public ::testing::Test
{
protected:
  // You can remove any or all of the following functions if their bodies would
  // be empty.

  PropanedbTest()
  {
    // You can do set-up work for each test here.
  }

  ~PropanedbTest() override
  {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  void SetUp() override
  {
    // Code here will be called immediately after the constructor (right
    // before each test).
    std::string dir("/tmp/test1");
    try
    {
      if (boost::filesystem::exists(dir))
      {
        boost::filesystem::remove_all(dir);
      }
      boost::filesystem::create_directory(dir);
    }
    catch (boost::filesystem::filesystem_error const &e)
    {
      //display error message
    }
    //DatabaseServiceImpl service(dir);
    service = new DatabaseServiceImpl(dir);
  }

  void TearDown() override
  {
    // Code here will be called immediately after each test (right
    // before the destructor).
    delete service;
  }

  // Class members declared here can be used by all tests in the test suite
  // for Foo.
  DatabaseServiceImpl *service;
};

TEST_F(PropanedbTest, PutGet)
{
  std::string id;

  test::TodoItem entity;
  string description("Test PropaneDB");
  entity.set_description(description);

  grpc::ServerContext context;
  context.AddInitialMetadata("database","test");
  {
    std::ifstream t("descriptor.bin");
    std::string descriptor((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());

    google::protobuf::FileDescriptorSet *fd = new google::protobuf::FileDescriptorSet;
    fd->ParseFromString(descriptor);
    LOG(INFO) << "Descriptor: " << fd->DebugString() << std::endl;

    propane::PropaneDatabase request;
    request.set_allocated_descriptor_set(fd);
    propane::PropaneStatus reply;
    grpc::Status s = service->CreateDatabase(&context, &request, &reply);
  }

  {
    Any *anyMessage = new Any();
    anyMessage->PackFrom(entity);
    propane::PropaneEntity request;
    request.set_allocated_data(anyMessage);

    propane::PropaneId reply;
    grpc::Status s = service->Put(&context, &request, &reply);

    EXPECT_EQ(s.ok(), true);
    EXPECT_GT(reply.id().length(), 0);
    id = reply.id();
  }

  {
    propane::PropaneId request;
    request.set_id(id);

    propane::PropaneEntity reply;
    grpc::Status s = service->Get(&context, &request, &reply);

    EXPECT_EQ(s.ok(), true);
    LOG(INFO) << "Get: any receive: " << reply.data().DebugString() << std::endl;
    // EXPECT_GT(reply.id().length(), 0);
  }
}

TEST_F(PropanedbTest, PutSearch)
{
  std::string id;
  test::TodoItem entity1;
  string description1("Item1");
  entity1.set_description(description1);
  entity1.set_isdone(false);

  test::TodoItem entity2;
  string description2("Item2");
  entity2.set_description(description2);
  entity2.set_isdone(true);

  grpc::ServerContext context;
  {
    std::ifstream t("descriptor.bin");
    std::string descriptor((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());
    google::protobuf::FileDescriptorSet *fd = new google::protobuf::FileDescriptorSet;
    fd->ParseFromString(descriptor);
    LOG(INFO) << "Descriptor: " << fd->DebugString() << std::endl;

    propane::PropaneDatabase request;
    request.set_allocated_descriptor_set(fd);
    propane::PropaneStatus reply;
    grpc::Status s = service->CreateDatabase(&context, &request, &reply);
  }

  {
    Any *anyMessage = new Any();
    anyMessage->PackFrom(entity1);
    propane::PropaneEntity request;
    request.set_allocated_data(anyMessage);

    propane::PropaneId reply;
    grpc::Status s = service->Put(&context, &request, &reply);

    EXPECT_EQ(s.ok(), true);
    EXPECT_GT(reply.id().length(), 0);
    id = reply.id();
  }

  {
    Any *anyMessage = new Any();
    anyMessage->PackFrom(entity2);
    propane::PropaneEntity request;
    request.set_allocated_data(anyMessage);

    propane::PropaneId reply;
    grpc::Status s = service->Put(&context, &request, &reply);

    EXPECT_EQ(s.ok(), true);
    EXPECT_GT(reply.id().length(), 0);
    id = reply.id();
  }

  {
    propane::PropaneSearch request;
    request.set_query("isDone==true");
    request.set_entitytype("test.TodoItem");

    propane::PropaneEntities reply;
    grpc::Status s = service->Search(&context, &request, &reply);
    //cout << "Reply =" <<reply.entities_size();
    EXPECT_EQ(s.ok(), true);
    EXPECT_EQ(reply.entities_size(), 1);
  }
}

TEST_F(PropanedbTest, PutDelete)
{
  std::string id;

  test::TodoItem entity;
  string description("Test PropaneDB");
  entity.set_description(description);

  grpc::ServerContext context;
  {
    std::ifstream t("descriptor.bin");
    std::string descriptor((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());

    google::protobuf::FileDescriptorSet *fd = new google::protobuf::FileDescriptorSet;
    fd->ParseFromString(descriptor);
    LOG(INFO) << "Descriptor: " << fd->DebugString() << std::endl;

    propane::PropaneDatabase request;
    request.set_allocated_descriptor_set(fd);
    propane::PropaneStatus reply;
    grpc::Status s = service->CreateDatabase(&context, &request, &reply);
  }

  {
    Any *anyMessage = new Any();
    anyMessage->PackFrom(entity);
    propane::PropaneEntity request;
    request.set_allocated_data(anyMessage);

    propane::PropaneId reply;
    grpc::Status s = service->Put(&context, &request, &reply);

    EXPECT_EQ(s.ok(), true);
    EXPECT_GT(reply.id().length(), 0);
    id = reply.id();
  }

  //delete object by ID
  {
    propane::PropaneId request;
    request.set_id(id);

    propane::PropaneStatus reply;
    grpc::Status s = service->Delete(&context, &request, &reply);

    EXPECT_EQ(s.ok(), true);
  }

  //delete same object again: should give error
  {
    propane::PropaneId request;
    request.set_id(id);

    propane::PropaneStatus reply;
    grpc::Status s = service->Delete(&context, &request, &reply);

    EXPECT_EQ(s.ok(), false);
  }
}

int main(int argc, char **argv)
{
  FLAGS_logtostderr = 1;
  // FLAGS_logtostderr = 1;
  //FLAGS_log_dir = "./";
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}