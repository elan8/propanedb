#include <string>
#include <glog/logging.h>

#include "Query.h"


class QueryParser final
{
private:
public:
    QueryParser();
    ~QueryParser();
    Query parseQuery(std::string query);
};