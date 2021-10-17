#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <boost/uuid/uuid.hpp>           
#include <boost/uuid/uuid_generators.hpp> 
#include <boost/uuid/uuid_io.hpp>         

class Util
{
public:
  static std::string getTypeName(const std::string &s)
  {
    std::string token = s.substr(s.find("/") + 1);
    return token;
  }

  static std::string generateUUID()
  {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return to_string(uuid);
  }
};