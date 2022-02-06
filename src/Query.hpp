#pragma once
#include <iostream>
#include <string_view>
#include <string>
//#include <google/protobuf/any.pb.h>
#include <google/protobuf/dynamic_message.h>

class Query {
 public:
  enum ComparisonOperator {
    None,
    Equal,
    NotEqual,
    GreaterThen,
    GreatherThenEqual,
    SmallerThen,
    SmallerThenEqual,
    Star
  };

 private:
  bool error;
  std::string errorMessage;
  std::string queryName;
  std::string queryValue;
  Query::ComparisonOperator queryOp = None;

 public:
  void setName(const std::string& name);
  void setValue(const std::string& value);
  void setComparisonOperator(ComparisonOperator);

  Query();
  bool hasError();
  void setError(const std::string& message);
  void clearError();
  std::string getErrorMessage();
  bool isMatch(const google::protobuf::Descriptor* descriptor,
               google::protobuf::Message* message);
};