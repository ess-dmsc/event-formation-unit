#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>

#define BUFSIZE 16384

int main()
{
  uint16_t buffer[BUFSIZE];

  for (int i = 0; i < BUFSIZE; i++) {
     buffer[i] = 16; // fixme add as parameter 
  }

  int fd = open("output.raw", O_WRONLY|O_CREAT);

  write(fd, buffer, sizeof(buffer));

  close(fd); 
}
