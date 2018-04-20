/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of StatPublisher
///
//===----------------------------------------------------------------------===//

#include <common/StatPublisher.h>

const int bufferSize = 1000;

StatPublisher::StatPublisher(std::string ip, int port) {
  statdb = new TCPTransmitter(ip.c_str(), port);
}

void StatPublisher::publish(std::shared_ptr<Detector> detector) {
  char buffer[bufferSize];
  int unixtime = (int)time(NULL);

  for (int i = 1; i <= detector->statsize(); i++) {
    int len =
        sprintf(buffer, "%s %" PRIi64 " %d\n", detector->statname(i).c_str(),
                detector->statvalue(i), unixtime);
    statdb->senddata(buffer, len);
  }
}
