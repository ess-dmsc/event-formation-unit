/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/StatPublisher.h>

StatPublisher::StatPublisher(std::string ip, int port) {
  statdb = new TCPClient(ip.c_str(), port);
}

void StatPublisher::publish(std::shared_ptr<Detector> detector) {
  char buffer[1000];
  int unixtime = (int)time(NULL);

  for (int i = 1; i <= detector->statsize(); i++) {
    int len =
        sprintf(buffer, "%s %" PRIi64 " %d\n", detector->statname(i).c_str(),
                detector->statvalue(i), unixtime);
    statdb->senddata(buffer, len);
  }
}
