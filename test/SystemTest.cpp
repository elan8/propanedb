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
#include "DatabaseImpl.hpp"
#include <boost/filesystem.hpp>
#include <chrono>
#include <thread>
#include <filesystem>

#include "propanedb.grpc.pb.h"
#include "Client.hpp"

using std::filesystem::exists;
using namespace std;

class SystemTest : public ::testing::Test
{
protected:
  SystemTest()
  {
  }

  ~SystemTest() override
  {
  }

  void SetUp() override
  {
    id = Util::generateUUID();
    LOG(INFO) << "SystemTest: Set up" << std::endl;
    dir = "/var/rocksdb/test/" + id;
    boost::filesystem::create_directory(dir);

    string pathEnv = "PATH=" + dir;
    string program_name("server");
    char *envp[] = {(char *)"DEBUG=true", (char *)pathEnv.c_str(), 0};
    char *arg_list[] = {program_name.data(), nullptr};

    if (!exists(program_name))
    {
      cout << "Program file 'server' does not exist in current directory!\n";
      exit(EXIT_FAILURE);
    }

    child_pid = fork();
    if (child_pid == -1)
    {
      perror("fork");
      exit(EXIT_FAILURE);
    }
    else if (child_pid > 0)
    {
      cout << "spawn child with pid - " << child_pid << endl;
    }
    else
    {
      execve(program_name.c_str(), arg_list, envp);
      perror("execve");
      exit(EXIT_FAILURE);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    auto target_str = "0.0.0.0:50051";
    client = new Client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    LOG(INFO) << "SystemTest: Client created" << std::endl;
  }

  void TearDown() override
  {
    // You can do clean-up work that doesn't throw exceptions here.
    LOG(INFO) << "SystemTest: Stop server" << std::endl;
    std::cout << "Killing child pid: " << child_pid << std::endl;
    kill(child_pid, SIGKILL);
    delete client;

    try
    {
      if (boost::filesystem::exists(dir))
      {
        //LOG(INFO) << "Remove folder : /var/rocksdb/test" << std::endl;
        boost::filesystem::remove_all(dir);
      }
    }
    catch (boost::filesystem::filesystem_error const &e)
    {
      LOG(INFO) << "Error: " << e.what() << std::endl;
    }
  }

  std::string id;
  std::string dir;

  pid_t child_pid;
  Client *client;
};

// TEST_F(SystemTest, PutGet)
// {

//   LOG(INFO) << "SystemTest: Create database" << std::endl;
//   auto status = client->CreateDatabase();

//   std::string id;

//   test::TestEntity item;
//   string description("Test1");
//   item.set_description(description);

//   status = client->Put(item, &id);
//   LOG(INFO) << "SystemTest: ID=" << id << std::endl;

//   if (status.ok())
//   {
//     LOG(INFO) << "SystemTest: Get 1" << std::endl;
//     test::TestEntity item2;
//     status = client->Get(id, &item2);
//     EXPECT_EQ(item2.description(), "Test1");
//   }
//   else
//   {
//     EXPECT_EQ(status.ok(), true);
//     LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
//   }
// }

TEST_F(SystemTest, FindBooks)
{

  auto status = client->CreateDatabase();

  std::string id;

  test::Author author1;
  author1.set_lastname("Baldacci");
  author1.set_firstname("David");

  test::Book book1;
  book1.set_title("the collectors");
  book1.set_allocated_author(new test::Author(author1));

  test::Book book2;
  book2.set_title("hour game");
  book2.set_allocated_author(new test::Author(author1));

  status = client->Put((google::protobuf::Message *)&author1, &id);
  status = client->Put((google::protobuf::Message *)&book1, &id);

  if (status.ok())
  {
    LOG(INFO) << "SystemTest: Get book 1" << std::endl;
    test::Book book1a;
    status = client->Get(id, &book1a);
    EXPECT_EQ(book1a.title(), book1.title());
  }
  else
  {
    EXPECT_EQ(status.ok(), true);
    LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
  }

  status = client->Put((google::protobuf::Message*)&book2, &id);
    if (status.ok())
  {
    LOG(INFO) << "SystemTest: Get book 2" << std::endl;
    test::Book book2a;
    status = client->Get(id, &book2a);
    EXPECT_EQ(book2a.title(), book2.title());
  }
  else
  {
    EXPECT_EQ(status.ok(), true);
    LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
  }

    if (status.ok())
  {
    LOG(INFO) << "SystemTest: Find books" << std::endl;
    test::Book book;
   Any anyMessage ;
    anyMessage.PackFrom(book);
    propane::PropaneEntities* entities;

//get all books
  LOG(INFO) << "SystemTest: URL=" << anyMessage.type_url()<< std::endl;
    status = client->Search(anyMessage.type_url(),"*", entities);

    if (status.ok())
  {
    // LOG(INFO) << "SystemTest: Get book 2" << std::endl;
    // test::Book book2a;
    // status = client->Get(id, &book2a);
    // EXPECT_EQ(book2a.title(), book2.title());
  }
  else
  {
    //EXPECT_EQ(status.ok(), true);
    LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
  }

    //LOG(INFO) << "SystemTest: Entities" << std::endl;
    //auto entities2 = entities->entities_size()
     //LOG(INFO) << entities->entities_size() << std::endl;
    //EXPECT_EQ(book2a.title(), book2.title());
  }


}

// TEST_F(SystemTest, BackupRestore)
// {

//   LOG(INFO) << "SystemTest: Create database" << std::endl;
//   auto status = client->CreateDatabase();

//   std::string id;

//   test::TestEntity item;
//   string description("Test1");
//   item.set_description(description);

//   status = client->Put(item, &id);
//   LOG(INFO) << "SystemTest: ID=" << id << std::endl;

//   if (status.ok())
//   {
//     LOG(INFO) << "SystemTest: Get 1" << std::endl;
//     test::TestEntity item2;
//     status = client->Get(id, &item2);
//     EXPECT_EQ(item2.description(), "Test1");
//   }
//   else
//   {
//     EXPECT_EQ(status.ok(), true);
//     LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
//   }

//   if (status.ok())
//   {
//     LOG(INFO) << "SystemTest: Backup" << std::endl;
//     status = client->Backup();
//   }
//   else
//   {
//     EXPECT_EQ(status.ok(), true);
//     LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
//   }

//   if (status.ok())
//   {
//     LOG(INFO) << "SystemTest: Get 2" << std::endl;
//     test::TestEntity item3;
//     status = client->Get(id, &item3);
//     EXPECT_EQ(item3.description(), "Test1");
//   }
//   else
//   {
//     EXPECT_EQ(status.ok(), true);
//     LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
//   }

//   LOG(INFO) << "SystemTest: Delete entity with ID=" << id << std::endl;
//   status = client->Delete(id);

//   if (status.ok())
//   {
//     LOG(INFO) << "SystemTest: Item deleted" << std::endl;
//   }
//   else
//   {
//     EXPECT_EQ(status.ok(), true);
//     LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
//   }

//   //check that item is removed from database
//   LOG(INFO) << "SystemTest: Get removed item" << std::endl;
//   test::TestEntity item3;
//   auto status2 = client->Get(id, &item3);
//   EXPECT_EQ(status2.ok(), false);
//   //LOG(INFO) << "Get: error code" << status2.error_message() << std::endl;

//   if (status.ok())
//   {
//     LOG(INFO) << "SystemTest: Restore" << std::endl;
//     status = client->Restore();
//   }
//   else
//   {
//     EXPECT_EQ(status.ok(), true);
//     LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
//   }

//   // std::this_thread::sleep_for(std::chrono::milliseconds(6000));

//   LOG(INFO) << "SystemTest: Get " << id << std::endl;
//   test::TestEntity item4;
//   status = client->Get(id, &item4);
//   EXPECT_EQ(status.ok(), true);
//   if (status.ok())
//   {
//     EXPECT_EQ(item4.description(), "Test1");
//     // LOG(INFO) << "Test completed succesfully!" << std::endl;
//   }
//   else
//   {

//     LOG(INFO) << "Get: error code" << status.error_message() << std::endl;
//   }
// }

int main(int argc, char **argv)
{
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}