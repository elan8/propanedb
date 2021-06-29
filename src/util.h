#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

class Util
{
public:
  static std::string getTypeName(const std::string &s)
  {
    std::string token = s.substr(s.find("/")+1);
    return token;
  }
};