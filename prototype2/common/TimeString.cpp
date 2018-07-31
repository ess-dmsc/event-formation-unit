/** Copyright (C) 2017 European Spallation Source ERIC */

#include <common/TimeString.h>

std::string timeString() {
  char cStartTime[50];
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(cStartTime, 50, "%Y-%m-%d-%H-%M-%S", timeinfo);
  std::string startTime = cStartTime;
  return startTime;
}


