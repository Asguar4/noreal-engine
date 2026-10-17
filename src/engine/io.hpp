#pragma once

#include <cstddef>
#include <string>

namespace riz {
  class File {
    char * data = nullptr;
    std::size_t size = 0;
    bool isValid = false;
  public:
    File(const std::string& path);
    File(const File& other);
    File(File&& other) noexcept;
    ~File();

    File& operator=(const File& other);
    File& operator=(File&& other) noexcept;
   
    const char * getData() const;
    std::size_t getSize() const;
    bool valid() const;
  };
}
