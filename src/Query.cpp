#include "Query.hpp"
#include <glog/logging.h>

Query::Query(): error(true), errorMessage(),queryName(),queryValue()
{
    // error = false;
    // errorMessage = "";
    // queryName = "";
    // queryValue = "";
    this->setError("Query not succesfully parsed.");
}

void Query::setName(const std::string& name)
{
    queryName = name;
}
void Query::setValue(const std::string& value)
{
    queryValue = value;
}

void Query::setComparisonOperator(ComparisonOperator op)
{
    queryOp = op;
}

void Query::setError(const std::string& message)
{
    error = true;
    errorMessage = message;
}

void Query::clearError()
{
    error = false;
    errorMessage = "";
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
     LOG(INFO) << "isMatch" << std::endl;
    bool output = false;

  if (error)
    {
        return false;
    }

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
         LOG(INFO) << "Query : bool " << std::endl;
        bool value = reflection->GetBool(*message, fd);
        bool desiredValue = false;
 
        const std::string& q = queryValue;
        if ( q.find("true")!=std::string::npos)
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
        queryValue.erase(remove( queryValue.begin(), queryValue.end(), '\'' ),queryValue.end());
         LOG(INFO) << "Query string : value="<< value << std::endl;
         LOG(INFO) << "Query string : queryValue="<< queryValue << std::endl;
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