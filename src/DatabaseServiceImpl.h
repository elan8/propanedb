#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <google/protobuf/dynamic_message.h>
#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
#include "propanedb.grpc.pb.h"
#include "util.h"
//#include "DatabaseServiceImpl.h"

using google::protobuf::Any;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using propane::Database;
using propane::PropaneEntity;
using propane::PropaneFileDescriptor;
using propane::PropaneId;
using propane::PropaneStatus;

using namespace ROCKSDB_NAMESPACE;
using namespace std;

class DatabaseServiceImpl final : public Database::Service
{
private:
    DB *db;
    google::protobuf::SimpleDescriptorDatabase *descriptorDB;

public:
    DatabaseServiceImpl(string path);
    ~DatabaseServiceImpl();
    grpc::Status Put(ServerContext *context, const PropaneEntity *request,
                     PropaneId *reply) override;
    grpc::Status Get(ServerContext *context, const PropaneId *request,
                     PropaneEntity *reply) override;
    grpc::Status SetFileDescriptor(ServerContext *context, const PropaneFileDescriptor *request,
                                   PropaneStatus *reply) override;
};