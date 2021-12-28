#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <boost/uuid/uuid.hpp>           
#include <boost/uuid/uuid_generators.hpp> 
#include <boost/uuid/uuid_io.hpp>     

#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include <ostream>
#include <stdexcept>
#include <algorithm>

#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace
{

    template <typename T>
    class MMapPtr : public std::unique_ptr<T, std::function<void(T *)>>
    {
    public:
        MMapPtr(T *addr, size_t len, int fd = -1) : std::unique_ptr<T, std::function<void(T *)>>(addr, [len, fd](T *addr)
                                                                                                 { unmap_and_close(addr, len, fd); })
        {
        }

        MMapPtr() : MMapPtr(nullptr, 0, -1) {}

        using std::unique_ptr<T, std::function<void(T *)>>::unique_ptr;
        using std::unique_ptr<T, std::function<void(T *)>>::operator=;

    private:
        static void unmap_and_close(const void *addr, size_t len, int fd)
        {
            if ((MAP_FAILED != addr) && (nullptr != addr) && (len > 0))
            {
                munmap(const_cast<void *>(addr), len);
            }

            if (fd >= 0)
            {
                close(fd);
            }

            return;
        }
    };
}; // Anonymous namespace

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