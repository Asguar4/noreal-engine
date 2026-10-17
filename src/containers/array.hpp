#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace riz {
  // Elem must be default-constructible and copy/move-assignable (storage is new Elem[cap]).
  template< class Elem >
  class Vector {
  public:
    using size_type = std::size_t;
  private:
    Elem * array;
    size_type allocated;
    size_type used;

    void reallocate(size_type newCap);
  public:
    Vector() noexcept;
    Vector(const Vector& other);
    Vector(Vector&& other) noexcept;
    Vector(std::initializer_list< Elem > init);

    ~Vector();

    Vector& operator=(const Vector& other);
    Vector& operator=(Vector&& other) noexcept;

    Elem& operator[](size_type index);
    const Elem& operator[](size_type index) const;

    void reserve(size_type newCap);
    void shrinkToFit();
    void resize(size_type count);
    void resize(size_type count, const Elem& value);

    size_type maxSize() const noexcept;
    size_type capacity() const noexcept;
    size_type size() const noexcept;
  };

  template< class Elem >
  void Vector<Elem>::reallocate(size_type newCap) {
    std::unique_ptr<Elem[]> newArray;
    if (newCap > 0)
      newArray.reset(new Elem[newCap]);

    const size_type n = std::min(used, newCap);
    for (size_type i = 0; i < n; ++i)
      newArray[i] = std::move_if_noexcept(array[i]);

    delete[] array;
    array = newArray.release();
    allocated = newCap;
    used = n;
  }

  template< class Elem >
  Vector<Elem>::Vector() noexcept : array(nullptr), allocated(0), used(0) {}

  template< class Elem >
  Vector<Elem>::Vector(const Vector& other) : array(nullptr), allocated(0), used(0) {
    std::unique_ptr<Elem[]> newArray;
    if (other.used > 0)
      newArray.reset(new Elem[other.used]);
    for (size_type i = 0; i < other.used; ++i)
      newArray[i] = other.array[i];
    array = newArray.release();
    allocated = other.used;
    used = other.used;
  }

  template< class Elem >
  Vector<Elem>::Vector(Vector&& other) noexcept
    : array(other.array), allocated(other.allocated), used(other.used) {
    other.array = nullptr;
    other.allocated = 0;
    other.used = 0;
  }

  template< class Elem >
  Vector<Elem>::Vector(std::initializer_list< Elem > init) : array(nullptr), allocated(0), used(0) {
    std::unique_ptr<Elem[]> newArray;
    if (init.size() > 0)
      newArray.reset(new Elem[init.size()]);
    size_type i = 0;
    for (const Elem& e : init)
      newArray[i++] = e;
    array = newArray.release();
    allocated = init.size();
    used = init.size();
  }

  template< class Elem >
  Vector<Elem>::~Vector() {
    delete[] array;
  }

  template< class Elem >
  Vector<Elem>& Vector<Elem>::operator=(const Vector& other) {
    if (this != &other) {
      std::unique_ptr<Elem[]> newArray;
      if (other.used > 0)
        newArray.reset(new Elem[other.used]);
      for (size_type i = 0; i < other.used; ++i)
        newArray[i] = other.array[i];

      delete[] array;
      array = newArray.release();
      allocated = other.used;
      used = other.used;
    }
    return *this;
  }

  template< class Elem >
  Vector<Elem>& Vector<Elem>::operator=(Vector&& other) noexcept {
    if (this != &other) {
      delete[] array;
      array = other.array;
      allocated = other.allocated;
      used = other.used;
      other.array = nullptr;
      other.allocated = 0;
      other.used = 0;
    }
    return *this;
  }

  template< class Elem >
  Elem& Vector<Elem>::operator[](size_type index) {
    assert(index < used);
    return array[index];
  }

  template< class Elem >
  const Elem& Vector<Elem>::operator[](size_type index) const {
    assert(index < used);
    return array[index];
  }

  template< class Elem >
  void Vector<Elem>::reserve(size_type newCap) {
    if (newCap > maxSize())
      throw std::length_error("Vector too large");
    if (newCap > allocated)
      reallocate(newCap);
  }

  template< class Elem >
  void Vector<Elem>::shrinkToFit() {
    if (used < allocated)
      reallocate(used);
  }

  template< class Elem >
  void Vector<Elem>::resize(size_type count) {
    resize(count, Elem{});
  }

  template< class Elem >
  void Vector<Elem>::resize(size_type count, const Elem& value) {
    if (count > used) {
      // value may alias an element of this vector: copy before reallocation
      const Elem copy = value;
      reserve(count);
      for (size_type i = used; i < count; ++i)
        array[i] = copy;
    }
    used = count;
  }

  template< class Elem >
  typename Vector<Elem>::size_type Vector<Elem>::maxSize() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(Elem);
  }

  template< class Elem >
  typename Vector<Elem>::size_type Vector<Elem>::capacity() const noexcept {
    return allocated;
  }

  template< class Elem >
  typename Vector<Elem>::size_type Vector<Elem>::size() const noexcept {
    return used;
  }
}
