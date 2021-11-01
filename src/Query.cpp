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
        if (queryOp == Query::Equal)
        {
            if (value == desiredValue)
            {
                output = true;
            }
        }
        else if (queryOp == Query::NotEqual)
        {
            if (value != desiredValue)
            {
                output = true;
            }
        }
        else
        {
            setError("This query operator is not allowed");
        }

        break;
    }

    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
    {
        std::string value = reflection->GetString(*message, fd);
        if (queryOp == Query::Equal)
        {
            if (value.compare(queryValue) == 0)
            {
                output = true;
            }
        }
        else if (queryOp == Query::NotEqual)
        {
            if (value.compare(queryValue) != 0)
            {
                output = true;
            }
        }
        else
        {
            setError("This query operator is not allowed");
        }

        break;
    }

    case google::protobuf::FieldDescriptor::TYPE_INT32:
    {
        int32_t value = reflection->GetInt32(*message, fd);
        if (value == strtol(queryValue.c_str(), NULL, 0))
        {
            output = true;
        }

        long v = strtol(queryValue.c_str(), NULL, 0);
        if (queryOp == Query::Equal)
        {
            if (value == v)
            {
                output = true;
            }
        }
        else if (queryOp == Query::NotEqual)
        {
            if (value != v)
            {
                output = true;
            }
        }
        else
        {
            setError("This query operator is not allowed");
        }

        break;
    }

    case google::protobuf::FieldDescriptor::TYPE_INT64:
    {
        int32_t value = reflection->GetInt64(*message, fd);
        if (value == strtol(queryValue.c_str(), NULL, 0))
        {
            output = true;
        }
        break;
    }

    case google::protobuf::FieldDescriptor::TYPE_UINT32:
    {
        int32_t value = reflection->GetUInt32(*message, fd);
        if (value == strtol(queryValue.c_str(), NULL, 0))
        {
            output = true;
        }
        break;
    }

    case google::protobuf::FieldDescriptor::TYPE_UINT64:
    {
        int32_t value = reflection->GetUInt64(*message, fd);
        if (value == strtol(queryValue.c_str(), NULL, 0))
        {
            output = true;
        }
        break;
    }

    default:
    {
        setError("This field cannot be used in search queries because of its datatype: " + queryName + "(" + fd->type_name() + ")");
        break;
    }
    }
    return output;
}