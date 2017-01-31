/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Simple RingBuffer class to keep track of a number of buffers
 *  for receiving socket data. User writes to buffers directly, so it is
 *  possible to write beyond buffers. However overwrites are detected on
 *  nextbuffer() and getdatabuffer() calls.
 */

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

  /** @brief construct a ringbuffer of specified size
   *  @param entries Maximum number of entries in ring
   */
  RingBuffer(const int entries);

  /** @brief minimal destructor frees the allocated buffer */
  ~RingBuffer();

  /** @brief Get the index of current active buffer
   * This function should only called by the Producer.
   */
  unsigned int getdataindex();

  /** @brief Get pointer to data for specified buffer
   * @param index Index of specified buffer
   */
  char *getdatabuffer(unsigned int index);

  /** @brief  Set length of available data in specified buffer, this
   * function is only called by Producer.
   *  @param index Index of the specified buffer
   *  @param length Size of data (Bytes)
   */
  void setdatalength(unsigned int index, unsigned int length);

  /** @brief get the length of data in specified  buffer
   *  @param index Index of specified buffer
   */
  int getdatalength(const unsigned int index);

  /** @brief  Advance to next buffer in ringbuffer, updated internal
   * data, checks for buffer overwrites, wraps around to first buffer.
   * Only called by Producer.
   */
  int nextbuffer();

  int getmaxbufsize() { return N; }          /**< return buffer size in bytes */
  int getmaxelems() { return max_entries_; } /**< return number of buffers */
  int getindex() { return entry_; }          /** current buffer index */

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

template <const unsigned int N> unsigned int RingBuffer<N>::getdataindex() {
  return entry_;
}

template <const unsigned int N>
char *RingBuffer<N>::getdatabuffer(unsigned int index) {
  assert(index < max_entries_);
  assert(data[index].cookie1 == COOKIE1);
  assert(data[index].cookie2 == COOKIE2);
  return data[index].buffer;
}

template <const unsigned int N>
int RingBuffer<N>::getdatalength(unsigned int index) {
  assert(index < max_entries_);
  return data[index].length;
}

template <const unsigned int N>
void RingBuffer<N>::setdatalength(unsigned int index, unsigned int length) {
  assert(length <= N);
  assert(length > 0);
  assert(index < max_entries_);
  data[index].length = length;
}

/** @todo using powers of two and bitmask in stead of modulus */
template <const unsigned int N> int RingBuffer<N>::nextbuffer() {
  entry_ = (entry_ + 1) % max_entries_;
  assert(data[entry_].cookie1 == COOKIE1);
  assert(data[entry_].cookie2 == COOKIE2);
  return entry_;
}
