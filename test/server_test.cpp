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
//#include <unistd.h>
#include <boost/filesystem.hpp>

using namespace std;

// Demonstrate some basic assertions.
TEST(HelloTest, PutGet)
{

  std::string dir("/tmp/test1");
  try
  {
    if (boost::filesystem::exists(dir))
    {
      boost::filesystem::remove_all(dir);
    }
    //create_directory(directory_path);
  }
  catch (boost::filesystem::filesystem_error const &e)
  {
    //display error message
  }

  std::string id;
  DatabaseServiceImpl service(dir);

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
    std::cout << "Descriptor: " << fd->DebugString() << std::endl;

    propane::PropaneFileDescriptor request;
    request.set_allocated_descriptor_set(fd);
    propane::PropaneStatus reply;
    grpc::Status s = service.SetFileDescriptor(&context, &request, &reply);
  }

  {
    Any *anyMessage = new Any();
    anyMessage->PackFrom(entity);
    propane::PropaneEntity request;
    request.set_allocated_data(anyMessage);

    propane::PropaneId reply;
    grpc::Status s = service.Put(&context, &request, &reply);

    EXPECT_EQ(s.ok(), true);
    EXPECT_GT(reply.id().length(), 0);
    id = reply.id();
  }

  {
    propane::PropaneId request;
    request.set_id(id);

    propane::PropaneEntity reply;
    grpc::Status s = service.Get(&context, &request, &reply);

    EXPECT_EQ(s.ok(), true);
     std::cout << "Get: any receive: " << reply.data().DebugString() << std::endl;
   // EXPECT_GT(reply.id().length(), 0);
  }
}