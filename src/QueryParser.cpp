#include "QueryParser.hpp"
#include "Query.hpp"
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/trace.hpp>
//#include <tao/pegtl/contrib/icu/utf8.hpp>

using namespace tao::pegtl;

namespace application {
// struct fieldname : seq< one<'\''> , plus<sor<alnum,blank>>,one<'\''>  >
struct fieldname : plus<alnum> {};

struct value
    : seq<one<'\''>, plus<sor<alnum, blank>>, one<'\''>>  // plus<alnum>
{};

struct equal : two<'='> {};

struct notequal : seq<one<'!'>, one<'='>> {};

struct gte : seq<one<'='>, one<'>'>> {};
struct gt : one<'>'> {};

struct star : one<'*'> {};

struct op : sor<equal, gt, gte> {};

struct expression : sor<seq<fieldname, op, value>, star> {};

struct grammar : seq<expression, eof> {};

template <typename Rule>
struct my_action : tao::pegtl::nothing<Rule> {};

template <>
struct my_action<application::star> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, Query &out) {
    // LOG(INFO) << "Operator = STAR" << std::endl;

    out.setComparisonOperator(Query::Star);
  }
};

template <>
struct my_action<application::equal> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, Query &out) {
    // LOG(INFO) << "Operator = EQUAL" << std::endl;

    out.setComparisonOperator(Query::Equal);
  }
};

template <>
struct my_action<application::notequal> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, Query &out) {
    // LOG(INFO) << "Operator = NOT EQUAL" << std::endl;

    out.setComparisonOperator(Query::NotEqual);
  }
};

template <>
struct my_action<application::expression> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, Query &out) {
    // LOG(INFO) << "Action = expression" << std::endl;
  }
};

template <>
struct my_action<application::grammar> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, Query &out) {
    // LOG(INFO) << "Action = grammar:" << in.string() << std::endl;

    out.clearError();
  }
};

template <>
struct my_action<application::fieldname> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, Query &out) {
    // LOG(INFO) << "Action = fieldname: " << in.string() << std::endl;

    out.setName(in.string());
  }
};

template <>
struct my_action<application::value> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, Query &out) {
    // LOG(INFO) << "Action = value: " << in.string() << std::endl;

    out.setValue(in.string());
  }
};

}  // namespace application

QueryParser::QueryParser(bool debug) { this->debug = debug; }

QueryParser::~QueryParser() {}

Query QueryParser::parseQuery(std::string queryString) {
  Query query;
  if (analyze<application::grammar>() != 0) {
    if (debug) {
      LOG(INFO) << "Grammar incorrect" << std::endl;
    }
    query.setError("Grammar incorrect");
    return query;
  }
  if (debug) {
    LOG(INFO) << "parseQuery: " << queryString << std::endl;
  }
  memory_input in1(queryString, "");
  tao::pegtl::parse<application::grammar, application::my_action>(in1, query);
  return query;
}