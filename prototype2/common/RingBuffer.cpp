/** Copyright (C) 2016 European Spallation Source ERIC */

#include <cassert>
#include <common/RingBuffer.h>
#include <cstdlib>

RingBuffer::RingBuffer(int entries) : N_(entries) {
  data = new Data[entries];
}

RingBuffer::~RingBuffer() {
  delete(data);
  data = 0;
}

struct RingBuffer::Data *RingBuffer::getdatastruct(void) {
  return &data[entry_];
}

void RingBuffer::setdatalength(int length) {
  assert(length <= size_);
  assert(length > 0);
  data[entry_].length = length;
}

int RingBuffer::getdatalength(void) { return data[entry_].length; }

int RingBuffer::nextbuffer(void) {
  entry_ = (entry_ + 1) % N_;
  return entry_;
}
