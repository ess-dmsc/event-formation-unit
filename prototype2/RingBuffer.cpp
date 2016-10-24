/** Copyright (C) 2016 European Spallation Source */

#include <RingBuffer.h>
#include <cassert>
#include <cstdlib>

RingBuffer::RingBuffer(int buffersize, int entries)
    : N_(entries), size_(buffersize) {

  buffers = (char *)malloc(N_ * size_);
  assert(buffers != 0);
}

char *RingBuffer::getbuffer(void) { return &buffers[entry_ * size_]; }

int RingBuffer::nextbuffer(void) {
  entry_ = (entry_ + 1) % N_;
  return entry_;
}
