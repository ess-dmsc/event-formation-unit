/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Simple RingBuffer class to keep track of a number of buffers
 *  for receiving socket data. No bounds checking so it is possible to corrupt
 *  data and write beyond buffers
 *  @todo add cookies around  each buffer
 */

#pragma once

class RingBuffer {
public:
  struct Data {
    int length;
    char buffer[9000]; /**< @todo Hardcoded buffersize */
  };

  /** @brief construct a ringbuffer of specified size
   *  @param entries size of the ringbuffer (in units of elements)
   */
  RingBuffer(int entries);

  /** @brief minimal destructor frees the allocated buffer */
  ~RingBuffer();

  struct Data *getdatastruct(); /**< return pointer to current buffer */
  void setdatalength(int length);   /**< specify length of data in curr buffer */
  int getdatalength(); /**< get the length of data in current buffer */
  int nextbuffer(); /**< advance to next buffer, wraps around */
  int getsize() { return size_; }   /**< return buffer size in bytes */
  int getelems() { return N_; }     /**< return number of buffers */
  int getindex() { return entry_; } /** current buffer index */

private:
  struct Data *data{nullptr};

  int entry_{0};
  int N_{0};
  int size_{9000}; /**< @todo  hardcoded must be in sync with buffer above */
};
