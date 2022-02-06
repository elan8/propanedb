#pragma once

#include <fstream>
#include <string>
#include <glog/logging.h>

class FileWriter {
 public:
  FileWriter();
  FileWriter(FileWriter&&);
  FileWriter& operator=(FileWriter&&);
  void OpenIfNecessary(const std::string& name);
  void Write(std::string& data);

  bool NoSpaceLeft() const { return m_no_space; }

 private:
  std::string m_name;
  std::ofstream m_ofs;
  bool m_no_space;

  void RaiseError [[noreturn]] (const std::string& action_attempted,
                                const std::system_error& ex);
};