#include "Query.h"

Query::Query()
{
    error = false;
    errorMessage = "";
    fieldName = "";
    fieldValue = "";
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
    //std::string fieldName = this->fieldName;

    const google::protobuf::FieldDescriptor *fd = descriptor->FindFieldByName(fieldName);
    const google::protobuf::Reflection *reflection = message->GetReflection();

    google::protobuf::FieldDescriptor::CppType type = fd->cpp_type();

    switch (type)
    {
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
    {
        bool value = reflection->GetBool(*message, fd);
        if (value == true)
        {
            output = true;
        }
        break;
    }

    default:
        break;
    }
    // if (fd->default_value_bool()){
    //     bool value = reflection->GetBool(*message, fd);
    //     if (value==true){
    //         output=true;
    //     }
    // }
    //string id = reflection->GetString(*message, fd);

    // LOG(INFO) << "isMatch:" <<  << endl;
    return output;
}