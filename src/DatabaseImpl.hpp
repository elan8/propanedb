#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <map>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <google/protobuf/dynamic_message.h>
#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
#include "propanedb.grpc.pb.h"
#include "util.h"
#include <glog/logging.h>
#include "QueryParser.hpp"

using google::protobuf::Any;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using propane::Database;

using namespace ROCKSDB_NAMESPACE;
using namespace std;

struct Metadata
{
    string databaseName;
};

class DatabaseImpl
{
private:
    string databasePath;
    string backupPath;
    google::protobuf::SimpleDescriptorDatabase *descriptorDB;
    const google::protobuf::DescriptorPool *pool;
    google::protobuf::DynamicMessageFactory dmf;
    QueryParser *queryParser;
    map<string, rocksdb::DB *> databases;
    bool debug;

    static bool IsCorrectEntityType(google::protobuf::Any *any, std::string type);
    rocksdb::DB *GetDatabase(string name);

public:
    DatabaseImpl(const string &path, bool debug);
    ~DatabaseImpl();
    void setDebugMode(bool enabled);
    grpc::Status Put(Metadata *metadata, const propane::PropanePut *request,
                     propane::PropaneId *reply);
    grpc::Status Get(Metadata *metadata, const propane::PropaneId *request,
                     propane::PropaneEntity *reply);
    grpc::Status Delete(Metadata *metadata, const propane::PropaneId *request,
                        propane::PropaneStatus *reply);
    grpc::Status Search(Metadata *metadata, const propane::PropaneSearch *request,
                        propane::PropaneEntities *reply);
    grpc::Status CreateDatabase(Metadata *metadata, const propane::PropaneDatabase *request,
                                propane::PropaneStatus *reply);

    grpc::Status Backup(Metadata *metadata, const string databaseName, string *zipFilePath);
    grpc::Status Restore(Metadata *metadata, const string databaseName, string zipFilePath);
};