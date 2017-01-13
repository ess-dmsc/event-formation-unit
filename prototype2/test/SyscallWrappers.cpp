/** Copyright (C) 2016 European Spallation Source ERIC */

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {

int forcefstatfail = 0;
int forcereadfail = 0;
int forcewritefail = 0;
int forceopenfail = 0;

int __real_open(char *pathname, int flags, int mode);
int __real_fstat(int fd, struct stat *buf);
int __real_read(int fd, void *buf, size_t count);
int __real_write(int fd, void *buf, size_t count);

int __wrap_open(char *pathname, int flags, int mode) {
  if (forceopenfail == 1) {
    printf("Forcing open() to fail\n");
    forceopenfail = 0;
    return -1;
  } else {
    if (forceopenfail > 0) {
      forceopenfail--;
    }
    return __real_open(pathname, flags, mode);
  }
}

int __wrap_fstat(int fd, struct stat *buf) {
  if (forcefstatfail) {
    printf("Forcing fstat() to fail\n");
    forcefstatfail = 0;
    return -1;
  } else {
    return __real_fstat(fd, buf);
  }
}

int __wrap_read(int fd, void *buf, size_t count) {
  if (forcereadfail) {
    printf("Forcing read() to fail\n");
    forcereadfail = 0;
    return -1;
  } else {
    return __real_read(fd, buf, count);
  }
}

int __wrap_write(int fd, void *buf, size_t count) {
  if (forcewritefail) {
    printf("Forcing write() to fail\n");
    forcewritefail = 0;
    return -1;
  } else {
    return __real_write(fd, buf, count);
  }
}

} // extern C
