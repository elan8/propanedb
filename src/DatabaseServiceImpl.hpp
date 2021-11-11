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
//#include "util.h"
#include <glog/logging.h>

#include "QueryParser.hpp"
#include "DatabaseImpl.hpp"

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
    string databasePath;
    string backupPath;
    DatabaseImpl *implementation;
    Metadata GetMetadata(ServerContext *context);
    bool debug;
public:
    DatabaseServiceImpl(const string &databasePath, const string &backupPath, bool debug);
    ~DatabaseServiceImpl();
    grpc::Status Put(ServerContext *context, const propane::PropanePut *request,
                     propane::PropaneId *reply) override;
    grpc::Status Get(ServerContext *context, const propane::PropaneId *request,
                     propane::PropaneEntity *reply) override;
    grpc::Status Delete(ServerContext *context, const propane::PropaneId *request,
                        propane::PropaneStatus *reply) override;
    grpc::Status Search(ServerContext *context, const propane::PropaneSearch *request,
                        propane::PropaneEntities *reply) override;
    grpc::Status CreateDatabase(ServerContext *context, const propane::PropaneDatabase *request,
                                propane::PropaneStatus *reply) override;


    grpc::Status Backup(ServerContext* context, const ::propane::PropaneBackupRequest* request, 
    ::grpc::ServerWriter< ::propane::PropaneBackupReply>* writer);
    grpc::Status Restore(ServerContext* context, ::grpc::ServerReader< ::propane::PropaneRestoreRequest>* reader, 
    ::propane::PropaneRestoreReply* response);

};