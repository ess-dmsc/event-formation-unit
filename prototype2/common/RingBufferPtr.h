/** Copyright (C) 2016 European Spallation Source ERIC */

#include <cassert>
#include <cstdlib>

/** @file
 *
	 *  @brief Simple RingBuffer class to keep track of a number of buffers.
 * Similar to RingBuffer.h but uses a Fifo of pointers to struct Data
 * in stead of buffer indexes. Should currently be considered unsupported.
 */

#pragma once

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
   *  @param entries size of the ringbuffer (in units of elements)
   */
  RingBuffer(const int entries);

  /** @brief minimal destructor frees the allocated buffer */
  ~RingBuffer();

  struct Data *getdatastruct(); /**< return pointer to current buffer */

  void setdatalength(
      unsigned int length); /**< specify length of data in curr buffer */
  int getdatalength();      /**< get the length of data in current buffer */
  int nextbuffer();         /**< advance to next buffer, wraps around */
  int getsize() { return buffersize_; }   /**< return buffer size in bytes */
  int getelems() { return max_entries_; } /**< return number of buffers */
  int getindex() { return entry_; }       /** current buffer index */

private:
  struct Data *data{nullptr};

  unsigned int entry_{0};
  unsigned int max_entries_{0};
  unsigned int buffersize_{N};
};

template <const unsigned int N>
RingBuffer<N>::RingBuffer(int entries) : max_entries_(entries) {
  data = new Data[entries];
}

template <const unsigned int N> RingBuffer<N>::~RingBuffer() {
  delete[] data;
  data = 0;
}

/** @todo experimental */
template <const unsigned int N> unsigned int RingBuffer<N>::getdataindex() {
  return entry_;
}

/** @todo experimental */
template <const unsigned int N>
char *RingBuffer<N>::getdatabuffer(unsigned int index) {
  assert(index < max_entries_);
  assert(data[index].cookie1 == COOKIE1);
  assert(data[index].cookie2 == COOKIE2);
  return data[index].buffer;
}

template <const unsigned int N>
struct RingBuffer<N>::Data *RingBuffer<N>::getdatastruct() {
  assert(data[entry_].cookie1 == COOKIE1);
  assert(data[entry_].cookie2 == COOKIE2);
  return &data[entry_];
}

template <const unsigned int N>
void RingBuffer<N>::setdatalength(unsigned int length) {
  assert(length <= buffersize_);
  assert(length > 0);
  data[entry_].length = length;
}

template <const unsigned int N> int RingBuffer<N>::getdatalength() {
  return data[entry_].length;
}

/** @todo using powers of two and bitmask in stead of modulus */
template <const unsigned int N> int RingBuffer<N>::nextbuffer() {
  entry_ = (entry_ + 1) % max_entries_;
  assert(data[entry_].cookie1 == COOKIE1);
  assert(data[entry_].cookie2 == COOKIE2);
  return entry_;
}
