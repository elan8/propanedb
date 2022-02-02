#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <streambuf>
#include <string>
#include <thread>

#include "DatabaseImpl.hpp"
#include "test.pb.h"

using namespace std;

class UnitTest : public ::testing::Test
{
protected:
  UnitTest()
  {
    // You can do set-up work for each test here.
  }

  ~UnitTest() override
  {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  void SetUp() override
  {
   LOG(INFO) << "SetUp" << std::endl;
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
      LOG(INFO) << "Error: " << e.what() << std::endl;
    }
    db = new DatabaseImpl(dir, dir, true);
  }

  void TearDown() override
  {
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
     LOG(INFO) << "Teardown" << std::endl;
    delete db;


    
  }

  DatabaseImpl *db{};

};

TEST_F(UnitTest, PutGet)
{
  std::string id;

  test::TestEntity item;
  string description("Test PropaneDB");
  item.set_description(description);

  Metadata meta;
  meta.databaseName = "test";

  {
    std::ifstream t("descriptor.bin");
    std::string descriptor((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());

    google::protobuf::FileDescriptorSet *fd = new google::protobuf::FileDescriptorSet;
    fd->ParseFromString(descriptor);
    propane::PropaneDatabase request;
    request.set_allocated_descriptor_set(fd);
    request.set_databasename("test");
    propane::PropaneStatus reply;
    grpc::Status s = db->CreateDatabase(&meta, &request, &reply);
  }

  {
    propane::PropanePut request;
    propane::PropaneEntity *entity = new propane::PropaneEntity();
    Any *anyMessage = new Any();
    anyMessage->PackFrom(item);

    entity->set_allocated_data(anyMessage);
    request.set_allocated_entity(entity);
    request.set_databasename("test");
    propane::PropaneId reply;

    grpc::Status s = db->Put(&meta, &request, &reply);
    LOG(INFO) << "Put: Status=" << s.error_message() << std::endl;
    EXPECT_EQ(s.ok(), true);
    EXPECT_GT(reply.id().length(), 0);
    id = reply.id();
  }

  {
    propane::PropaneId request;
    request.set_id(id);
    request.set_databasename("test");

    propane::PropaneEntity reply;
    //Metadata meta;
    grpc::Status s = db->Get(&meta, &request, &reply);

    EXPECT_EQ(s.ok(), true);
    LOG(INFO) << "Get: any receive: " << reply.data().DebugString() << std::endl;
  }

 
}


TEST_F(UnitTest, SearchAll)
{
  std::string id;

  test::TestEntity item1;
  string description1("Test1");
  item1.set_description(description1);

  test::TestEntity item2;
  string description2("Test2");
  item2.set_description(description2);

  Metadata meta;
  meta.databaseName = "test";

  {
    std::ifstream t("descriptor.bin");
    std::string descriptor((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());

    google::protobuf::FileDescriptorSet *fd = new google::protobuf::FileDescriptorSet;
    fd->ParseFromString(descriptor);
    propane::PropaneDatabase request;
    request.set_allocated_descriptor_set(fd);
    request.set_databasename("test");
    propane::PropaneStatus reply;
    grpc::Status s = db->CreateDatabase(&meta, &request, &reply);
  }

  {
    propane::PropanePut request;
    propane::PropaneEntity *entity = new propane::PropaneEntity();
    Any *anyMessage = new Any();
    anyMessage->PackFrom(item1);

    entity->set_allocated_data(anyMessage);
    request.set_allocated_entity(entity);
    request.set_databasename("test");
    propane::PropaneId reply;

    grpc::Status s = db->Put(&meta, &request, &reply);
    LOG(INFO) << "Put: Status=" << s.error_message() << std::endl;
    EXPECT_EQ(s.ok(), true);
    EXPECT_GT(reply.id().length(), 0);
    id = reply.id();
  }

  {
    propane::PropanePut request;
    propane::PropaneEntity *entity = new propane::PropaneEntity();
    Any *anyMessage = new Any();
    anyMessage->PackFrom(item2);

    entity->set_allocated_data(anyMessage);
    request.set_allocated_entity(entity);
    request.set_databasename("test");
    propane::PropaneId reply;

    grpc::Status s = db->Put(&meta, &request, &reply);
    LOG(INFO) << "Put: Status=" << s.error_message() << std::endl;
    EXPECT_EQ(s.ok(), true);
    EXPECT_GT(reply.id().length(), 0);
    id = reply.id();
  }

  {
        Any *anyMessage = new Any();
    anyMessage->PackFrom(item1);

    propane::PropaneSearch request;
    request.set_entitytype(anyMessage->type_url());
    request.set_query("*");

    propane::PropaneEntities reply;
    //Metadata meta;
    grpc::Status s = db->Search(&meta, &request, &reply);

    EXPECT_EQ(s.ok(), true);
    if(! s.ok()){
      LOG(ERROR) << "Search: error= " << s.error_message() << std::endl;
    }
    EXPECT_EQ(reply.entities().size(), 2);
    LOG(INFO) << "Object 1= " << reply.entities()[0].DebugString()<< std::endl;
    LOG(INFO) << "Object 2= " << reply.entities()[1].DebugString()<< std::endl;
  }

 
}

auto main(int argc, char **argv) -> int {
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}