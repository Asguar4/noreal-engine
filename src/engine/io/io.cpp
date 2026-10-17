#include <cstddef>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>

#include "../io.hpp"

namespace riz {
  File::File(const std::string& path) {
    std::ifstream fp(path, std::ios::binary);
    if (!fp.is_open()) {
      throw std::runtime_error("Error reading file: " + path);
    }
    data.assign(std::istreambuf_iterator<char>(fp), std::istreambuf_iterator<char>());
  }

  File::File(File&& other) noexcept : data(std::move(other.data)) {
    other.data.clear();
  }

  File& File::operator=(File&& other) noexcept {
    if (this != &other) {
      data = std::move(other.data);
      other.data.clear();
    }
    return *this;
  }

  const char * File::getData() const noexcept {
    return data.c_str();
  }

  std::size_t File::getSize() const noexcept {
    return data.size();
  }
}
