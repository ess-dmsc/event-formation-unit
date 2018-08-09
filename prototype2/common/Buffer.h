/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
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

// \todo make specific types, maybe template?

template<typename T>
struct Buffer {
  Buffer() {}

  Buffer(T *address, size_t sz)
      : buffer(address), size(sz) {}

  Buffer(std::vector<T> &vector)
      : Buffer(vector.data(), vector.size()) {}

  operator bool() const {
    return (buffer && size);
  }

  // prefix ++
  Buffer& operator++ ()
  {
    ++buffer;
    --size;
    return *this;
  }

  // postfix ++
  Buffer operator++ (int)
  {
    Buffer result(*this);
    ++(*this);
    return result;
  }

  Buffer operator+=(const size_t& rhs)
  {
    this->buffer += rhs;
    this->size -= rhs;
    return *this;
  }

  T *buffer{nullptr};
  size_t size{0};
};

