#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

class Util
{
public:
  static std::string split(const std::string &s, std::string delimiter)
  {
    std::string token = s.substr(0, s.find(delimiter));
    return token;
  }
};