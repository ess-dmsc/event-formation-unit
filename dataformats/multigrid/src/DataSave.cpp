/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <DataSave.h>

DataSave::DataSave(std::string filename, void *buffer, size_t datasize) {

  if ((fd = open(filename.c_str(), flags, mode)) < 0) {
    std::string msg = "DataSave: open(" + filename + ") failed";
    perror(msg.c_str());
  }

  int ret;
  if ((ret = write(fd, buffer, datasize)) < 0) {
    std::string msg = "DataSave write(" + filename + ") failed";
    perror(msg.c_str());
  }
}

DataSave::DataSave(std::string filename) {

  if ((fd = open(filename.c_str(), flags, mode)) < 0) {
    std::string msg = "DataSave: open(" + filename + ") failed";
    perror(msg.c_str());
  }
}

int DataSave::tofile(std::string text) {
  if (fd < 0)
    return -1;

  return write(fd, text.c_str(), text.size());
}

int DataSave::tofile(char *buffer, size_t len) {
  if (fd < 0)
    return -1;

  return write(fd, buffer, len);
}

DataSave::~DataSave() { close(fd); }
