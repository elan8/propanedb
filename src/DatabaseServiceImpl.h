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

#include "QueryParser.h"

using google::protobuf::Any;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using propane::Database;


using namespace ROCKSDB_NAMESPACE;
using namespace std;

class DatabaseServiceImpl final : public Database::Service
{
private:
    //DB *db;
    string directory;
    google::protobuf::SimpleDescriptorDatabase *descriptorDB;
    const google::protobuf::DescriptorPool *pool ;
    google::protobuf::DynamicMessageFactory dmf;
    QueryParser* queryParser;
    map<string, rocksdb::DB*> databases;

    static bool IsCorrectEntityType(google::protobuf::Any* any, std::string type );
    rocksdb::DB* GetDatabase(string name);
    //string GetDatabaseNameFromContext(grpc::ServerContext* context);

public:
    DatabaseServiceImpl(string path);
    ~DatabaseServiceImpl();
    grpc::Status Put(ServerContext *context, const propane::PropanePut *request,
                     propane::PropaneId *reply) override;
    grpc::Status Get(ServerContext *context, const propane::PropaneId *request,
                     propane::PropaneEntity *reply) override;
    grpc::Status Delete(ServerContext *context, const propane::PropaneId *request,
                        propane::PropaneStatus *reply) override;
    grpc::Status Search(ServerContext *context, const propane::PropaneSearch *request,
                        propane::PropaneEntities *reply) override;
    grpc::Status CreateDatabase(ServerContext *context, const propane::PropaneDatabase* request,
                                   propane::PropaneStatus *reply) override;
};