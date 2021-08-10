#include "Query.h"

Query::Query()
{
    error=false;
    errorMessage="";
}

void Query::setError(std::string message) 
{
    error=true;
    errorMessage=message;
}

bool Query::hasError()
{
    return error;
}

std::string Query::getErrorMessage()
{
    return errorMessage;
}

bool Query::isMatch(google::protobuf::Message* message)
{
     // LOG(INFO) << "isMatch:" <<  << endl;
    return true;
}