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
    LOG(INFO) << "SystemTest: Set up" << std::endl;
    std::string dir("/var/rocksdb/test");
    try
    {
      if (boost::filesystem::exists(dir))
      {
        LOG(INFO) << "Remove folder : /var/rocksdb/test" << std::endl;
        boost::filesystem::remove_all(dir);
      }
    }
    catch (boost::filesystem::filesystem_error const &e)
    {
      LOG(INFO) << "Error: " << e.what() << std::endl;
    }

    string program_name("server");
    char *envp[] = {(char *)"DEBUG=true", 0};
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

  ~SystemTest() override
  {
    // You can do clean-up work that doesn't throw exceptions here.
    LOG(INFO) << "SystemTest: Stop server" << std::endl;
    std::cout << "Killing child pid: " << child_pid << std::endl;
    kill(child_pid, SIGKILL);
    delete client;
  }

  void SetUp() override
  {
  }

  void TearDown() override
  {
  }

  pid_t child_pid;
  Client *client;
};

TEST_F(SystemTest, BackupRestore)
{

  LOG(INFO) << "SystemTest: Create database" << std::endl;
  auto status = client->CreateDatabase();

  std::string id;

  test::TestEntity item;
  string description("Test1");
  item.set_description(description);

  status = client->Put(item, &id);
  LOG(INFO) << "SystemTest: ID=" << id << std::endl;

  if (status.ok())
  {
    LOG(INFO) << "SystemTest: Get 1" << std::endl;
    test::TestEntity item2;
    status = client->Get(id, &item2);
    EXPECT_EQ(item2.description(), "Test1");
  }
  else
  {
    EXPECT_EQ(status.ok(), true);
    LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
  }

  if (status.ok())
  {
    LOG(INFO) << "SystemTest: Backup" << std::endl;
    status = client->Backup();
  }
  else
  {
    EXPECT_EQ(status.ok(), true);
    LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
  }

 if (status.ok())
  {
    LOG(INFO) << "SystemTest: Get 2" << std::endl;
    test::TestEntity item3;
    status = client->Get(id, &item3);
    EXPECT_EQ(item3.description(), "Test1");
  }
  else
  {
    EXPECT_EQ(status.ok(), true);
    LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
  }


  LOG(INFO) << "SystemTest: Delete entity with ID=" << id << std::endl;
  status = client->Delete(id);

  if (status.ok())
  {
    LOG(INFO) << "SystemTest: Item deleted" << std::endl;
  }
  else
  {
    EXPECT_EQ(status.ok(), true);
    LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
  }

  //check that item is removed from database
  LOG(INFO) << "SystemTest: Get removed item" << std::endl;
  test::TestEntity item3;
  auto status2 = client->Get(id, &item3);
  EXPECT_EQ(status2.ok(), false);
  //LOG(INFO) << "Get: error code" << status2.error_message() << std::endl;

  if (status.ok())
  {
    LOG(INFO) << "SystemTest: Restore" << std::endl;
    status = client->Restore();
   
  }
  else
  {
    EXPECT_EQ(status.ok(), true);
    LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(6000));

  LOG(INFO) << "SystemTest: Get " << id << std::endl;
  test::TestEntity item4;
  status = client->Get(id, &item4);
  EXPECT_EQ(status.ok(), true);
  if (status.ok())
  {
    EXPECT_EQ(item4.description(), "Test1");
    // LOG(INFO) << "Test completed succesfully!" << std::endl;
  }
  else
  {

    LOG(INFO) << "Get: error code" << status.error_message() << std::endl;
  }
}

int main(int argc, char **argv)
{
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}