#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#include "../io.hpp"

static char * resize(char * data, const size_t used, const size_t size) {
  char * result = new char[size];
  for (size_t i = 0; i < std::min(size, used); ++i) {
    result[i] = data[i];
  }
  delete[] data;

  return result;
} 

namespace riz {
  File::File(const std::string& path) {
    data = new char[1];

    std::ifstream fp(path, std::ios::binary);
    if (!fp.is_open()) {
      std::cerr << "error reading file: " << path << std::endl;
      return;
    }

    const size_t chunkSize = 1024 * 1024;
    char * tmp = new char[chunkSize];

    while (fp.read(tmp, chunkSize) || fp.gcount() > 0) {
      const size_t used = static_cast<size_t>(fp.gcount());
      data = resize(data, size, size + used + 1);
      for (size_t i = size; i < size + used; ++i) {
        data[i] = tmp[i - size];
      }
      size += used;
    }
    delete[] tmp;

    data[size] = '\0';
    isValid = true;
  }

  File::File(const File& other) {
    data = new char[other.size + 1];
    for (size_t i = 0; i < other.size; ++i) {
      data[i] = other.data[i];
    }
    data[other.size] = '\0';
    size = other.size;
    isValid = other.isValid;
  }

  File::File(File&& other) {
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
      char * newData = new char[other.size + 1];
      for (size_t i = 0; i < other.size; ++i) {
        newData[i] = other.data[i];
      }
      newData[other.size] = '\0';

      delete[] data;
      data = newData;
      size = other.size;
      isValid = other.isValid;
    }
    return *this;
  }

  File& File::operator=(File&& other) {
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
    return data;
  }

  size_t File::getSize() const {
    return size;
  }

  bool File::valid() const {
    return isValid;
  }
}
