/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <prototype2/common/Trace.h>
#include <dataformats/multigrid/inc/DataSave.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

DataSave::DataSave(std::string filename, void *buffer, size_t datasize):
   filename_prefix(filename) {

  if ((fd = open(filename_prefix.c_str(), flags, mode)) < 0) {
    std::string msg = "DataSave: open(" + filename + ") failed";
    perror(msg.c_str());
  }

  int ret;
  if ((ret = write(fd, buffer, datasize)) < 0) {
    std::string msg = "DataSave write(" + filename + ") failed";
    perror(msg.c_str());
  }
}

DataSave::DataSave(std::string filename): filename_prefix(filename) {

  if ((fd = open(filename_prefix.c_str(), flags, mode)) < 0) {
    std::string msg = "DataSave: open(" + filename + ") failed";
    perror(msg.c_str());
  }
}

DataSave::DataSave(std::string name, int maxlen) :
  filename_prefix(name), maxfilesize(maxlen) {

  char cStartTime[50];
  time_t rawtime;
  struct tm * timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(cStartTime, 50, "%Y%m%d-%H%M%S", timeinfo);
  startTime = cStartTime;
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
  if (fd < 0)
    return -1;

  int retlen = 0;
  va_list args;
  va_start(args, fmt);
  /** @brief prevent buffer overrun, but data could be truncated */
  auto maxwritelen = BUFFERSIZE + MARGIN - bufferlen;
  int ret = vsnprintf(buffer + bufferlen, maxwritelen, fmt, args);
  if (ret < 0) {
    XTRACE(PROCESS, ERR, "vsnprintf failed\n");
    return ret;
  }
  va_end(args);

  if (ret > maxwritelen) {
    XTRACE(PROCESS, WAR, "datasave has been truncated\n");
    ret = maxwritelen;
  }

  bufferlen += ret;
  if (bufferlen >= BUFFERSIZE) {
    XTRACE(PROCESS, DEB, "Writing chunk of size %d\n", bufferlen);
    write(fd, buffer, bufferlen);
    retlen  = bufferlen;
    bufferlen = 0;
  }
  return adjustfilesize(retlen);
}

DataSave::~DataSave() {
  XTRACE(PROCESS, DEB, "~DataSave bufferlen %d\n", bufferlen);
  if (bufferlen > 0) {
    XTRACE(PROCESS, INF, "Flushing DataSave buffer\n");
    write(fd, buffer, bufferlen);
  }
  close(fd);
}

/** Private functions below, API functions above */

std::string DataSave::getfilename() {
  return filename_prefix + startTime + "_" + std::to_string(sequence_number - 1) + ".csv";
}

void DataSave::createfile() {
  curfilesize=0;

  close(fd);

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
