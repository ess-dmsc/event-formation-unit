// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief common definition of buffer, used as reference to some location in
///        memory with a length. NOT responsible for (de-)allocation!
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <cstddef>
#include <vector>

// \todo reverse iteration facilities

template <typename T> struct Buffer {

  // \brief creates invalid buffer
  Buffer() {}

  // \brief creates buffer with provided address and size
  Buffer(T *addr, size_t sz) : address(addr), size(sz) {}

  // \brief creates buffer as reference to standard vector
  Buffer(std::vector<T> &vector) : Buffer(vector.data(), vector.size()) {}

  size_t bytes() const { return size * sizeof(T); }

  // \brief creates buffer from buffer, converting bit resolution
  template <typename TT>
  explicit Buffer(Buffer<TT> other)
      : address(reinterpret_cast<T *>(other.address)),
        size(other.bytes() / sizeof(T)) {}

  // \brief checks buffer validity
  operator bool() const { return (address && size); }

  // \brief access buffer element by value
  T operator[](size_t i) const { return address[i]; }

  // \brief access buffer element by reference
  T &operator[](size_t i) { return address[i]; }

  // \brief access buffer element by const reference
  const T &at(size_t i) const { return address[i]; }

  // \brief access first element by value
  T operator*() const { return *address; }

  // \brief access first element by reference
  T &operator*() { return *address; }

  // \brief prefix increment
  Buffer &operator++() {
    ++address;
    --size;
    return *this;
  }

  // \brief postfix increment
  Buffer operator++(int) {
    Buffer result(*this);
    ++(*this);
    return result;
  }

  // \brief increment by value
  Buffer operator+=(const size_t &rhs) {
    this->address += rhs;
    this->size -= rhs;
    return *this;
  }

  T *address{nullptr};
  size_t size{0};
};
