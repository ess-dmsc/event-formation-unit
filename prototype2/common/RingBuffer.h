/** Copyright (C) 2016 European Spallation Source */

#pragma once

// Simple class to keep track of anumber of buffers
// for receiving socket data. No bounds checking
// So it is possible to corrupt data and write beyond
// buffers (TODO add cookies around  each buffer)

class RingBuffer {
public:
  struct Data {
    int length;
    char buffer[9000]; // FIXME hardcoded
  };

  RingBuffer(int entries);
  ~RingBuffer();
  struct Data *getdatastruct(void); /**< return pointer to current buffer */
  void setdatalength(int length);   /**< set the length field of curr buff */
  int getdatalength(void);
  int nextbuffer(void); /**< advance to next buffer, wraps around */
  int getsize(void) { return size_; }   /**< return buffer size in bytes */
  int getelems(void) { return N_; }     /**< return number of buffers */
  int getindex(void) { return entry_; } /** current buffer index */

private:
  struct Data *data{nullptr};

  int entry_{0};
  int N_{0};
  int size_{9000}; // FIXME hardcoded must be in sync with buffer above
};
