/** Copyright (C) 2016 European Spallation Source */

#include <cassert>
#include <common/RingBuffer.h>
#include <cstdlib>

RingBuffer::RingBuffer(int entries) : N_(entries) {

  data = (struct Data *)malloc(sizeof(struct Data) * entries);

  assert(data != 0);
}

RingBuffer::~RingBuffer() {
  free(data);
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
