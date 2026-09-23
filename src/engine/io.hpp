#pragma once

#include <string>

namespace riz {
  class File {
    char * data = nullptr;
    size_t size = 0;
    bool isValid = false;
  public:
    File(const std::string& path);
    File(const File& other);
    File(File&& other);
    ~File();

    File& operator=(const File& other);
    File& operator=(File&& other);
   
    const char * getData() const;
    size_t getSize() const;
    bool valid() const;
  };
}
