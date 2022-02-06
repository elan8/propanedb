#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>

namespace {

template <typename T>
class MMapPtr : public std::unique_ptr<T, std::function<void(T *)>> {
 public:
  MMapPtr(T *addr, size_t len, int fd = -1)
      : std::unique_ptr<T, std::function<void(T *)>>(
            addr, [len, fd](T *addr) { unmap_and_close(addr, len, fd); }) {}

  MMapPtr() : MMapPtr(nullptr, 0, -1) {}

  using std::unique_ptr<T, std::function<void(T *)>>::unique_ptr;
  using std::unique_ptr<T, std::function<void(T *)>>::operator=;

 private:
  static void unmap_and_close(const void *addr, size_t len, int fd) {
    if ((MAP_FAILED != addr) && (nullptr != addr) && (len > 0)) {
      munmap(const_cast<void *>(addr), len);
    }

    if (fd >= 0) {
      close(fd);
    }

    return;
  }
};
};  // Anonymous namespace

class Util {
 public:
  static std::string getTypeName(const std::string &s) {
    std::string token = s.substr(s.find("/") + 1);
    return token;
  }

  static std::string generateUUID() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return to_string(uuid);
  }

  static std::string getAbsolutePath(std::string path, std::string fileName) {
    std::filesystem::path p1;
    p1 += path;
    p1 /= fileName;
    return p1.generic_string();
  }

  static bool pathExists(std::string path) {
    return std::filesystem::exists(path);
  }
};