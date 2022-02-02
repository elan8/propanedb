#pragma once

#include <string>
#include <glog/logging.h>
#include "Query.hpp"

class QueryParser final
{
private:
    bool debug;
public:
    explicit QueryParser(bool debug);
    ~QueryParser();
    Query parseQuery(std::string query);
};