#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../io.hpp"

namespace {
  const char kEmpty[1] = { '\0' };
}

namespace riz {
  File::File(const std::string& path) {
    std::ifstream fp(path, std::ios::binary);
    if (!fp.is_open()) {
      std::cerr << "error reading file: " << path << std::endl;
      return;
    }

    const std::size_t chunkSize = 1024 * 1024;
    std::unique_ptr<char[]> tmp(new char[chunkSize]);
    std::vector<char> content;

    while (fp.read(tmp.get(), chunkSize) || fp.gcount() > 0) {
      const std::size_t used = static_cast<std::size_t>(fp.gcount());
      content.insert(content.end(), tmp.get(), tmp.get() + used);
    }

    std::unique_ptr<char[]> buf(new char[content.size() + 1]);
    for (std::size_t i = 0; i < content.size(); ++i) {
      buf[i] = content[i];
    }
    buf[content.size()] = '\0';

    size = content.size();
    data = buf.release();
    isValid = true;
  }

  File::File(const File& other) {
    if (other.data) {
      std::unique_ptr<char[]> buf(new char[other.size + 1]);
      for (std::size_t i = 0; i < other.size; ++i) {
        buf[i] = other.data[i];
      }
      buf[other.size] = '\0';
      data = buf.release();
    }
    size = other.size;
    isValid = other.isValid;
  }

  File::File(File&& other) noexcept {
    data = other.data;
    size = other.size;
    isValid = other.isValid;
    other.data = nullptr;
    other.size = 0;
    other.isValid = false;
  }

  File::~File() {
    delete[] data;
  }

  File& File::operator=(const File& other) {
    if (this != &other) {
      char * newData = nullptr;
      if (other.data) {
        std::unique_ptr<char[]> buf(new char[other.size + 1]);
        for (std::size_t i = 0; i < other.size; ++i) {
          buf[i] = other.data[i];
        }
        buf[other.size] = '\0';
        newData = buf.release();
      }

      delete[] data;
      data = newData;
      size = other.size;
      isValid = other.isValid;
    }
    return *this;
  }

  File& File::operator=(File&& other) noexcept {
    if (this != &other) {
      delete[] data;
      data = other.data;
      size = other.size;
      isValid = other.isValid;
      other.data = nullptr;
      other.size = 0;
      other.isValid = false;
    }
    return *this;
  }

  const char * File::getData() const {
    return data ? data : kEmpty;
  }

  std::size_t File::getSize() const {
    return size;
  }

  bool File::valid() const {
    return isValid;
  }
}
