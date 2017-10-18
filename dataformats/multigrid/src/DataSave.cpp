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

DataSave::DataSave(std::string filename, int __attribute__((unused)) unixtime) :
    filename_prefix(filename)
{
   createfile();
}


int DataSave::tofile(char *buffer, size_t len) {
  if (fd < 0)
    return -1;

  return adjustfilesize(write(fd, buffer, len));
}

int DataSave::tofile(std::string text) {
  return tofile(text.c_str(), text.size());
}

int DataSave::tofile(const char * fmt,...) {
  char buffer[2000];
  if (fd < 0)
    return -1;
  va_list args;
  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);
  buffer[1999] = 0;
  return adjustfilesize(dprintf(fd, "%s", buffer));
}

DataSave::~DataSave() { close(fd); }

/** Private functions below, API functions above */

void DataSave::createfile() {
  curfilesize=0;

  close(fd);
  char cStartTime[50];
  time_t rawtime;
  struct tm * timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(cStartTime, 50, "%Y%m%d-%H%M%S", timeinfo);
  std::string startTime = cStartTime;
  std::string fileName = filename_prefix + startTime + "_" + std::to_string(sequence_number) + ".csv";

  if ((fd = open(fileName.c_str(), flags, mode)) < 0) {
    std::string msg = "DataSave: open(" + fileName + ") failed";
    perror(msg.c_str());
  }

  sequence_number++;
}

// Helper function
int DataSave::adjustfilesize(int returnval) {
    if (returnval > 0) {
       curfilesize+=returnval;
    }
    if (curfilesize >= maxfilesize) {
      createfile();
    }
    return returnval;
}
