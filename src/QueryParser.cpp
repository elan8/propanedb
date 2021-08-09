#include "QueryParser.h"
#include <tao/pegtl.hpp>

// Include the analyze function that checks
// a grammar for possible infinite cycles.

#include <tao/pegtl/contrib/analyze.hpp>


using namespace tao::pegtl;

namespace application
{



    struct fieldname : plus<alnum>
    {
    };

    struct equals : two<'='>
    {
    };
    struct gt : seq<one<'='>, one<'>'>>
    {
    };

    struct grammar : seq<fieldname, equals, eof>
    {
    };
}
// struct comment
//     : seq<one<'#'>, until<eolf>>
// {
// };




QueryParser::QueryParser()
{
}

QueryParser::~QueryParser()
{
}

Query QueryParser::parseQuery(std::string queryString)
{
    Query query;
    if (analyze<application::grammar>() != 0)
    {
        query.setError("Grammar incorrect");
        return query;
    }

      LOG(INFO) << "parseQuery: " << queryString << std::endl;

    return query;
}