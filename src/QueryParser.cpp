#include "QueryParser.h"
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/trace.hpp>

using namespace tao::pegtl;

namespace application
{

    struct fieldname : plus<alnum>
    {
    };

    struct value : plus<alnum>
    {
    };

    struct equals : two<'='>
    {
    };
    struct gte : seq<one<'='>, one<'>'>>
    {
    };
    struct gt : one<'>'>
    {
    };

    struct op : sor<equals, gt, gte>
    {
    };

    struct grammar : seq<fieldname, op, value, eof>
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
        LOG(INFO) << "Grammar incorrect" << std::endl;
        query.setError("Grammar incorrect");
        return query;
    }

    LOG(INFO) << "parseQuery: " << queryString << std::endl;

    memory_input in1(queryString, "");
    tao::pegtl::standard_trace<application::grammar>(in1);

    return query;
}