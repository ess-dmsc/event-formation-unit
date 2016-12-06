
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

extern "C" {

int forcefstatfail = 0;
int forcereadfail = 0;

int __real_fstat(int fd, struct stat * buf);
int __real_read(int fd, void * buf, size_t count);

int __wrap_fstat(int fd, struct stat * buf) {
  if (forcefstatfail) {
    printf("Forcing fstat() to fail\n");
    forcefstatfail=0;
    return -1;
  } else {
    return __real_fstat(fd, buf);
  }
}

int __wrap_read(int fd, void * buf, size_t count) {
    if (forcereadfail) {
      printf("Forcing read() to fail\n");
      forcereadfail=0;
      return -1;
    } else {
      return __real_read(fd, buf, count);
    }
}

} // extern C
