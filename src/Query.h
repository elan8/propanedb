//#include <google/protobuf/any.pb.h>
#include <google/protobuf/dynamic_message.h>

enum ComparisonOperator { Equal, NotEqual, GreaterThen, GreatherThenEqual, SmallerThen, SmallerThenEqual };

class Query
{
private:
    bool error;
    std::string errorMessage;


public:

    std::string fieldName;
    std::string fieldValue;

    Query();
    bool hasError();
    void setError(std::string message);
    std::string getErrorMessage();
    bool isMatch(const google::protobuf::Descriptor *descriptor ,google::protobuf::Message *message);
};