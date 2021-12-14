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
    // You can do set-up work for each test here.

    std::string dir("/var/rocksdb/test");
    try
    {
      if (boost::filesystem::exists(dir))
      {
          LOG(INFO) << "Remove folder : /var/rocksdb/test" << std::endl;
        boost::filesystem::remove_all(dir);
      }
      //boost::filesystem::create_directory(dir);
    }
    catch (boost::filesystem::filesystem_error const &e)
    {
      LOG(INFO) << "Error: " << e.what() << std::endl;
    }

  LOG(INFO) << "SystemTest: SetUp" << std::endl;
    //char *cmd = (char *)"./server";
    //char *arg[] = {NULL};

    string program_name("server");
    //string argument("debug");
    char *envp[] =
        {
            (char *)"DEBUG=true",

            0};
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
      //return child_pid;
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
        LOG(INFO) << "SystemTest: TearDown" << std::endl;
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
  std::string databaseName = "test";
  LOG(INFO) << "SystemTest: Create database" << std::endl;
  auto status = client->CreateDatabase(databaseName);

  if (status.ok())
  {
    LOG(INFO) << "SystemTest: Backup" << std::endl;
    client->Backup(databaseName);
  }
  else
  {
    LOG(INFO) << "SystemTest: error" << status.error_message() << std::endl;
  }

  //     //grpc::ClientContext context;
  //    // ClientContext context;
  //    //auto stub_ = propane::Database::NewStub()

  //     // Here we can use the stub's newly available method we just added.
  //     //grpc::Status status = stub_->SayHelloAgain(&context, request, &reply);
}

int main(int argc, char **argv)
{
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}