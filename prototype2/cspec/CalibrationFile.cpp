/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <common/Trace.h>
#include <cspec/CSPECChanConv.h>
#include <cspec/CalibrationFile.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

int CalibrationFile::load(std::string calibration, char *wirecal,
                          char *gridcal) {
  XTRACE(CMD, INF, "Attempt to load calibration %s\n", calibration.c_str());

  auto file = calibration + std::string(".wcal");
  if (load_file(file, wirecal) < 0) {
    return -1;
  }
  file = calibration + std::string(".gcal");
  if (load_file(file, gridcal) < 0) {
    return -1;
  }
  return 0;
}

int CalibrationFile::save(std::string calibration, char *wirecal,
                          char *gridcal) {
  XTRACE(CMD, INF, "Attempt to save calibration %s\n", calibration.c_str());

  auto file = calibration + std::string(".wcal");
  if (save_file(file, wirecal) < 0) {
    return -1;
  }
  file = calibration + std::string(".gcal");
  if (save_file(file, gridcal) < 0) {
    return -1;
  }
  return 0;
}

int CalibrationFile::load_file(std::string file, char *buffer) {
  struct stat buf;

  std::fill_n((char *)&buf, sizeof(struct stat), 0);

  int fd = open(file.c_str(), O_RDONLY);
  if (fd < 0) {
    XTRACE(CMD, WAR, "file open() failed for %s\n", file.c_str());
    return -10;
  }

  if (fstat(fd, &buf) != 0) {
    XTRACE(CMD, ERR, "fstat() failed for %s\n", file.c_str());
    close(fd);
    return -11;
  }

  if (buf.st_size != CSPECChanConv::adcsize * 2) {
    XTRACE(CMD, WAR, "file %s has wrong length: %d (should be %d)\n",
           file.c_str(), (int)buf.st_size, 16384 * 2);
    close(fd);
    return -12;
  }

  if (read(fd, buffer, CSPECChanConv::adcsize * 2) !=
      CSPECChanConv::adcsize * 2) {
    XTRACE(CMD, ERR, "read() from %s incomplete\n", file.c_str());
    close(fd);
    return -13;
  }
  XTRACE(CMD, INF, "Calibration file %s sucessfully read\n", file.c_str());
  close(fd);
  return 0;
}

int CalibrationFile::save_file(std::string file, char *buffer) {
  static const int flags = O_TRUNC | O_CREAT | O_WRONLY;
  static const int mode = S_IRUSR | S_IWUSR;

  int fd;

  if ((fd = open(file.c_str(), flags, mode)) < 0) {
    XTRACE(CMD, ERR, "open() %s failed\n", file.c_str());
    return -21;
  }
  if (write(fd, buffer, CSPECChanConv::adcsize * 2) !=
      CSPECChanConv::adcsize * 2) {
    XTRACE(CMD, ERR, "write() to %s incomplete\n", file.c_str());
    close(fd);
    return -22;
  }
  close(fd);
  return 0;
}
