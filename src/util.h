#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

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
    //std::cout << uuid << std::endl;
    return to_string(uuid);
  }
};