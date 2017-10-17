/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <dataformats/multigrid/inc/DataSave.h>

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

DataSave::DataSave(std::string filename_prefix, int __attribute__((unused)) unixtime) {
  char cStartTime[50];
  time_t rawtime;
  struct tm * timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(cStartTime, 50, "%Y-%m-%d-%H-%M-%S", timeinfo);
  std::string startTime = cStartTime;
  std::string fileName = filename_prefix + startTime + ".csv";

  if ((fd = open(fileName.c_str(), flags, mode)) < 0) {
    std::string msg = "DataSave: open(" + fileName + ") failed";
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

int DataSave::tofile(const char * fmt,...) {
  char buffer[200];
  if (fd < 0)
    return -1;
  va_list args;
  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);
  buffer[199] = 0;
  return dprintf(fd, "%s", buffer);

}

DataSave::~DataSave() { close(fd); }
