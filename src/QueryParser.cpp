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

     struct expression : seq<fieldname, op, value, eof>
    {
    };

    struct grammar : seq<expression, eof>
    {
    };

    // Primary action class template.
    template <typename Rule>
    struct my_action
        : tao::pegtl::nothing<Rule>
    {
    };

            template <>
    struct my_action<application::expression>
    {
        // Implement an apply() function that will be called by
        // the PEGTL every time tao::pegtl::any matches during
        // the parsing run.
        template <typename ActionInput>
        static void apply(const ActionInput &in, Query &out)
        {
            LOG(INFO) << "Action = expression" << std::endl;
            // Get the portion of the original input that the
            // rule matched this time as string and append it
            // to the result string.
            //out += in.string();
            //out.fieldName = in.string();
        }
    };

        template <>
    struct my_action<application::grammar>
    {
        // Implement an apply() function that will be called by
        // the PEGTL every time tao::pegtl::any matches during
        // the parsing run.
        template <typename ActionInput>
        static void apply(const ActionInput &in, Query &out)
        {
            LOG(INFO) << "Action = grammar" << std::endl;
            // Get the portion of the original input that the
            // rule matched this time as string and append it
            // to the result string.
            //out += in.string();
            //out.fieldName = in.string();
        }
    };

    // Specialise the action class template.
    template <>
    struct my_action<application::fieldname>
    {
        // Implement an apply() function that will be called by
        // the PEGTL every time tao::pegtl::any matches during
        // the parsing run.
        template <typename ActionInput>
        static void apply(const ActionInput &in, Query &out)
        {
            LOG(INFO) << "Action = fieldname" << std::endl;
            // Get the portion of the original input that the
            // rule matched this time as string and append it
            // to the result string.
            //out += in.string();
            out.fieldName = in.string();
        }
    };

    template <>
    struct my_action<application::value>
    {
        // Implement an apply() function that will be called by
        // the PEGTL every time tao::pegtl::any matches during
        // the parsing run.
        template <typename ActionInput>
        static void apply(const ActionInput &in, Query &out)
        {
              LOG(INFO) << "Action = value" << std::endl;
            // Get the portion of the original input that the
            // rule matched this time as string and append it
            // to the result string.
            //out += in.string();
            out.fieldValue = in.string();
        }
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
    //tao::pegtl::standard_trace<application::grammar>(in1);
    //in1.restart()
    tao::pegtl::parse<application::grammar, application::my_action>(in1, query);

    LOG(INFO) << "Query fieldName= " << query.fieldName << std::endl;
    LOG(INFO) << "Query fieldValue= " << query.fieldValue << std::endl;

    return query;
}