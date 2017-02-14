#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<inttypes.h>
#include<stdio.h>
#include<stdlib.h>

#define BUFSIZE 16384

int main(int argc, char * argv[])
{
  uint16_t buffer[BUFSIZE];
  if (argc != 2) {
    printf("usage: gencalibfile intvalue\n");
    exit(1);
  }
  int val = atoi(argv[1]);
  printf("Generating calibration file with constant value %d\n", val);
  

  for (int i = 0; i < BUFSIZE; i++) {
     buffer[i] = val;
  }

  int fd = open("output.raw", O_WRONLY|O_CREAT);

  write(fd, buffer, sizeof(buffer));

  close(fd); 
}
