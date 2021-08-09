//#include <google/protobuf/any.pb.h>
#include <google/protobuf/dynamic_message.h>

class Query
{
private:
    bool error;
    std::string errorMessage;

public:
    Query();
    bool hasError();
    void setError(std::string message);
    std::string getErrorMessage();
    bool isMatch(google::protobuf::Message *message);
};