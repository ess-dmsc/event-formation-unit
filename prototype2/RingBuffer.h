/** Copyright (C) 2016 European Spallation Source */

// Simple class to keep track of anumber of buffers
// for receiving socket data. No bounds checking
// So it is possible to corrupt data and write beyond
// buffers (TODO add cookies around  each buffer)

class RingBuffer {

public:
  RingBuffer(int buffersize, int entries);
  char *getbuffer(void); /**< return pointer to current buffer */
  int nextbuffer(void); /**< advance to next buffer, wraps around */
  int getsize(void) { return size_; } /**< return buffer size in bytes */
  int getelems(void) { return N_; } /**< return number of buffers */
  int getentry(void) {return entry_;} /** current buffer index */

private:
  char *buffers{nullptr};
  int entry_{0};
  int N_{0};
  int size_{0};
};
