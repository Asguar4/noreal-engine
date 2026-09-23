#pragma once

#include <initializer_list>
#include <algorithm>
#include <utility>
#include <stdexcept>

namespace riz {
  template< class Elem >
  class Array {
    Elem * array;
    size_t capacity;
    size_t used;
  public:
    Array();
    Array(size_t initSize);
    Array(std::initializer_list< Elem > init);
    Array(const Array& other);
    Array(Array&& other) noexcept;

    ~Array();

    Array& operator=(const Array& other);
    Array& operator=(Array&& other) noexcept;

    Elem& operator[](size_t index);
    const Elem& operator[](size_t index) const;

    void insert(const Elem& val);
    void insert(Elem&& val);
    void resize(size_t newSize);

    size_t size() const noexcept;
  };

  template< class Elem >
  Array<Elem>::Array() : array(nullptr), capacity(0), used(0) {}

  template< class Elem >
  Array<Elem>::Array(size_t initSize) {
    array = new Elem[initSize];
    capacity = initSize;
    used = 0;
  }

  template< class Elem >
  Array<Elem>::Array(std::initializer_list< Elem > init) {
    array = new Elem[init.size()];
    capacity = init.size();
    used = 0;
    for (auto i = init.begin(); i != init.end(); ++i) {
      array[used++] = *i;
    }
  }

  template< class Elem >
  Array<Elem>::Array(const Array& other) {
    array = new Elem[other.capacity];
    used = 0;
    while (used < other.used) {
      array[used] = other[used];
      ++used;
    }
    capacity = other.capacity;
  }

  template< class Elem >
  Array<Elem>::Array(Array&& other) noexcept {
    array = other.array;
    capacity = other.capacity;
    used = other.used;
    other.array = nullptr;
    other.capacity = 0;
    other.used = 0;
  }

  template< class Elem >
  Array<Elem>::~Array() {
    delete[] array;
  }

  template< class Elem >
  Array<Elem>& Array<Elem>::operator=(const Array& other) {
    if (this != &other) {
      Elem * newArray = new Elem[other.capacity];
      size_t newUsed = 0;
      while (newUsed < other.used) {
        newArray[newUsed] = other[newUsed];
        ++newUsed;
      }
      delete[] array;

      array = newArray;
      capacity = other.capacity;
      used = newUsed;
    }
    return *this;
  }

  template< class Elem >
  Array<Elem>& Array<Elem>::operator=(Array&& other) noexcept {
    if (this != &other) {
      delete[] array;
      array = other.array;
      capacity = other.capacity;
      used = other.used;
      other.array = nullptr;
      other.capacity = 0;
      other.used = 0;
    }
    return *this;
  }

  template< class Elem >
  Elem& Array<Elem>::operator[](size_t index) {
    if (index >= used) {
      throw std::out_of_range("Array index out of range");
    }
    return *(array + index);
  }

  template< class Elem>
  const Elem& Array<Elem>::operator[](size_t index) const {
    if (index >= used) {
      throw std::out_of_range("Array index out of range");
    }
    return *(array + index);
  }

  template< class Elem >
  void Array<Elem>::insert(const Elem& val) {
    if (used >= capacity) {
      throw std::out_of_range("Array is full");
    }
    array[used++] = val;
  }

  template< class Elem >
  void Array<Elem>::insert(Elem&& val) {
    if (used >= capacity) {
      throw std::out_of_range("Array is full");
    }
    array[used++] = std::move(val);
  }

  template< class Elem >
  void Array<Elem>::resize(size_t newSize) {
    Elem * newArray = new Elem[newSize];
    size_t newUsed = std::min(used, newSize);
    for (size_t i = 0; i < newUsed; ++i) {
      newArray[i] = std::move(array[i]);
    }
    delete[] array;

    array = newArray;
    capacity = newSize;
    used = newUsed;
  }

  template< class Elem >
  size_t Array<Elem>::size() const noexcept {
    return used;
  }
}
