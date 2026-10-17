#pragma once

#include <cstddef>
#include <string>

namespace riz {
  class File {
    std::string data;
  public:
    explicit File(const std::string& path);

    File(const File& other) = default;
    File(File&& other) noexcept;
    ~File() = default;

    File& operator=(const File& other) = default;
    File& operator=(File&& other) noexcept;

    const char * getData() const noexcept;
    std::size_t getSize() const noexcept;
  };
}
