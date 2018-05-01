/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

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
#include <stdexcept>

MapFile::MapFile(std::string filename) {
  struct stat sb;

  if ((fd = open(filename.c_str(), O_RDONLY)) < 0) {
    perror("open() failed");
    exit(1);
  }

  auto res = fstat(fd, &sb);
  if (res == -1)
    throw(std::runtime_error("MapFile assert 1 failed"));

  filesize = sb.st_size;
  address = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
  if (address == MAP_FAILED)
    throw(std::runtime_error("MapFile assert 2 failed"));
}

MapFile::~MapFile() {
  munmap(address, filesize);
//  auto res = munmap(address, filesize);
//  if (res != 0)
//    throw(std::runtime_error("MapFile assert 3 failed"));
  close(fd);
}

const char *MapFile::getaddress() { return (const char *)address; }

size_t MapFile::getsize() { return filesize; }
