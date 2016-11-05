/** Copyright (C) 2016 European Spallation Source ERIC */

#include <cassert>
#include <cstdlib>

/** @file
 *
 *  @brief Simple RingBuffer class to keep track of a number of buffers
 *  for receiving socket data. No bounds checking so it is possible to corrupt
 *  data and write beyond buffers
 *  @todo add cookies around  each buffer
 */

#pragma once

template <const unsigned int N>
class RingBuffer {
public:
  struct Data {
    int length;
    char buffer[N];
  };

  /** @brief construct a ringbuffer of specified size
   *  @param entries size of the ringbuffer (in units of elements)
   */
  RingBuffer(int entries);

  /** @brief minimal destructor frees the allocated buffer */
  ~RingBuffer();

  struct Data *getdatastruct();   /**< return pointer to current buffer */
  void setdatalength(int length); /**< specify length of data in curr buffer */
  int getdatalength(); /**< get the length of data in current buffer */
  int nextbuffer();    /**< advance to next buffer, wraps around */
  int getsize() { return size_; }   /**< return buffer size in bytes */
  int getelems() { return N_; }     /**< return number of buffers */
  int getindex() { return entry_; } /** current buffer index */

private:
  struct Data *data{nullptr};

  int entry_{0};
  int N_{0};
  int size_{N};
};

template <const unsigned int N> RingBuffer<N>::RingBuffer(int entries) : N_(entries) { data = new Data[entries]; }

template <const unsigned int N> RingBuffer<N>::~RingBuffer() {
  delete[] data;
  data = 0;
}

template <const unsigned int N> struct RingBuffer<N>::Data *RingBuffer<N>::getdatastruct() {
  return &data[entry_];
}

template <const unsigned int N> void RingBuffer<N>::setdatalength(int length) {
  assert(length <= size_);
  assert(length > 0);
  data[entry_].length = length;
}

template <const unsigned int N> int RingBuffer<N>::getdatalength() { return data[entry_].length; }

template <const unsigned int N> int RingBuffer<N>::nextbuffer() {
  entry_ = (entry_ + 1) % N_;
  return entry_;
}
