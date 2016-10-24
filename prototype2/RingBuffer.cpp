/** Copyright (C) 2016 European Spallation Source */

#include <RingBuffer.h>
#include <cassert>
#include <cstdlib>

RingBuffer::RingBuffer(int buffersize, int entries)
    : N_(entries), size_(buffersize) {

  data = (struct Data *)malloc(sizeof(struct Data) * entries);

  assert(data != 0);
}

struct RingBuffer::Data *RingBuffer::getdatastruct(void) {
  return &data[entry_];
}

void RingBuffer::setdatalength(int length) { data[entry_].length = length; }

int RingBuffer::nextbuffer(void) {
  entry_ = (entry_ + 1) % N_;
  return entry_;
}
