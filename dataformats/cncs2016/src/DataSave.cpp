/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <DataSave.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

DataSave::DataSave(std::string filename, void *buffer, size_t datasize) {
  const int flags = O_TRUNC | O_CREAT | O_WRONLY;
  const int mode = S_IRUSR | S_IWUSR;

  if ((fd = open(filename.c_str(), flags, mode)) < 0) {
    std::string msg = "DataSave: open(" + filename + ") failed";
    perror(msg.c_str());
  }

  int ret;
  if ((ret = write(fd, buffer, datasize)) < 0) {
    std::string msg = "DataSave: write(" + filename + ") failed";
    perror(msg.c_str());
  }
}

DataSave::~DataSave() { close(fd); }
