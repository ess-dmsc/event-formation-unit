// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Simple RingBuffer class to keep track of a number of buffers
/// for receiving socket data.
///
/// User writes to buffers directly, so it is possible to write beyond buffers.
/// However overwrites can be checked using verifyBufferCookies() if paranoid.
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>
#include <cstdlib>

template <const unsigned int N> class RingBuffer {
  static const unsigned int COOKIE1 = 0xDEADC0DE;
  static const unsigned int COOKIE2 = 0xFEE1DEAD;

public:
  struct Data {
    unsigned int cookie1 = COOKIE1;
    int length;
    char buffer[N];
    unsigned int cookie2 = COOKIE2;
  };

  /// \brief construct a ringbuffer of specified size
  /// \param entries Maximum number of entries in ring
  RingBuffer(const int entries);

  /// \brief minimal destructor frees the allocated buffer
  ~RingBuffer();

  /// \brief Get the index of current active buffer
  /// This function should only called by the Producer.
  unsigned int getDataIndex();

  /// \brief Get pointer to data for specified buffer
  /// \param index Index of specified buffer
  char *getDataBuffer(unsigned int index);

  /// \brief  Set length of available data in specified buffer, this
  /// function is only called by Producer.
  /// \param index Index of the specified buffer
  /// \param length Size of data (Bytes)
  void setDataLength(unsigned int index, unsigned int length);

  /// \brief get the length of data in specified  buffer
  /// \param index Index of specified buffer
  int getDataLength(const unsigned int index);

  /// \brief  Advance to next buffer in ringbuffer, updated internal
  /// data, checks for buffer overwrites, wraps around to first buffer.
  /// Only called by Producer.
  int getNextBuffer();

  int getMaxBufSize() { return N; }             ///< return buffer size in bytes
  int getMaxElements() { return max_entries_; } ///< return number of buffers

  bool verifyBufferCookies(unsigned int index) {
    return (data[index].cookie1 == COOKIE1 && data[index].cookie2 == COOKIE2);
  };

private:
  struct Data *data{nullptr};
  unsigned int entry_{0};
  unsigned int max_entries_{0};
};

template <const unsigned int N>
RingBuffer<N>::RingBuffer(int entries) : max_entries_(entries) {
  data = new Data[entries];
}

template <const unsigned int N> RingBuffer<N>::~RingBuffer() {
  delete[] data;
  data = 0;
}

template <const unsigned int N> unsigned int RingBuffer<N>::getDataIndex() {
  return entry_;
}

template <const unsigned int N>
char *RingBuffer<N>::getDataBuffer(unsigned int index) {
  assert(index < max_entries_);
  assert(data[index].cookie1 == COOKIE1);
  assert(data[index].cookie2 == COOKIE2);
  return data[index].buffer;
}

template <const unsigned int N>
int RingBuffer<N>::getDataLength(unsigned int index) {
  assert(index < max_entries_);
  return data[index].length;
}

template <const unsigned int N>
void RingBuffer<N>::setDataLength(unsigned int index, unsigned int length) {
  assert(length <= N);
  assert(index < max_entries_);
  data[index].length = length;
}

/// \todo using powers of two and bitmask in stead of modulus
template <const unsigned int N> int RingBuffer<N>::getNextBuffer() {
  entry_ = (entry_ + 1) % max_entries_;
  return entry_;
}
