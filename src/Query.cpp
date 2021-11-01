#include "Query.hpp"

Query::Query()
{
    error = false;
    errorMessage = "";
    queryName = "";
    queryValue = "";
}

void Query::setName(std::string name)
{
    queryName = name;
}
void Query::setValue(std::string value)
{
    queryValue = value;
}

void Query::setComparisonOperator(ComparisonOperator op)
{
    queryOp = op;
}

void Query::setError(std::string message)
{
    error = true;
    errorMessage = message;
}

bool Query::hasError()
{
    return error;
}

std::string Query::getErrorMessage()
{
    return errorMessage;
}

bool Query::isMatch(const google::protobuf::Descriptor *descriptor, google::protobuf::Message *message)
{
    bool output = false;
    if (queryOp == Query::Star)
    {
        return true;
    }
    const google::protobuf::FieldDescriptor *fd = descriptor->FindFieldByName(queryName);
    const google::protobuf::Reflection *reflection = message->GetReflection();
    google::protobuf::FieldDescriptor::CppType type = fd->cpp_type();

    switch (type)
    {
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
    {
        bool value = reflection->GetBool(*message, fd);
        bool desiredValue = false;
        if (queryValue.find("true"))
        {
            desiredValue = true;
        }
        if (value == desiredValue)
        {
            output = true;
        }
        break;
    }

    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
    {
        std::string value = reflection->GetString(*message, fd);
        if (value.compare(queryValue) == 0)
        {
            output = true;
        }
        break;
    }

    default:
        break;
    }
    return output;
}