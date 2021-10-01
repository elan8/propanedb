#include "QueryParser.h"
#include "Query.h"
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

    struct equal : two<'='>
    {
    };

    struct notequal : seq<one<'!'>, one<'='>>
    {
    };

    struct gte : seq<one<'='>, one<'>'>>
    {
    };
    struct gt : one<'>'>
    {
    };

    struct star : one<'*'>
    {
    };

    struct op : sor<equal, gt, gte>
    {
    };

    struct expression : sor< seq<fieldname, op, value, eof>, star>
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
    struct my_action<application::star>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, Query &out)
        {
            LOG(INFO) << "Operator = STAR" << std::endl;
            //out.setFieldValue(in.string());
            out.setComparisonOperator(Query::Star);

        }
    };

        template <>
    struct my_action<application::equal>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, Query &out)
        {
            LOG(INFO) << "Operator = EQUAL" << std::endl;
            //out.setFieldValue(in.string());
            out.setComparisonOperator(Query::Equal);

        }
    };

        template <>
    struct my_action<application::notequal>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, Query &out)
        {
            LOG(INFO) << "Operator = NOT EQUAL" << std::endl;
            //out.setFieldValue(in.string());
            out.setComparisonOperator(Query::NotEqual);

        }
    };

    template <>
    struct my_action<application::expression>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, Query &out)
        {
            LOG(INFO) << "Action = expression" << std::endl;
        }
    };

    template <>
    struct my_action<application::grammar>
    {

        template <typename ActionInput>
        static void apply(const ActionInput &in, Query &out)
        {
            LOG(INFO) << "Action = grammar" << std::endl;
        }
    };

    template <>
    struct my_action<application::fieldname>
    {

        template <typename ActionInput>
        static void apply(const ActionInput &in, Query &out)
        {
            LOG(INFO) << "Action = fieldname" << std::endl;

            out.setName(in.string());
        }
    };

    template <>
    struct my_action<application::value>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, Query &out)
        {
            LOG(INFO) << "Action = value" << std::endl;
            out.setValue(in.string());
        }
    };

}

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

    //LOG(INFO) << "Query fieldName= " << query.fieldName << std::endl;
    //LOG(INFO) << "Query fieldValue= " << query.fieldValue << std::endl;

    return query;
}