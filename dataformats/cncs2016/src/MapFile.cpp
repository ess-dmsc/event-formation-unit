/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief simple histogram class
 */

#include <MapFile.h>
#include <cassert>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

MapFile::MapFile(std::string filename) {
  struct stat sb;

  if ((fd = open(filename.c_str(), O_RDONLY)) < 0) {
    perror("open() failed");
    exit(1);
  }

  auto res = fstat(fd, &sb);
  assert(res != -1);

  filesize = sb.st_size;
  address = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(address != MAP_FAILED);
}

MapFile::~MapFile() {
  auto res = munmap(address, filesize);
  assert(res == 0);
  close(fd);
}

const char *MapFile::getaddress() { return (const char *)address; }

size_t MapFile::getsize() { return filesize; }
