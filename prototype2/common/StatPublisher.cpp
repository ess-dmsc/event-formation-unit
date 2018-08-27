/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of StatPublisher
///
//===----------------------------------------------------------------------===//

#include <common/StatPublisher.h>
#include <common/Log.h>

const int bufferSize = 1000;

///
StatPublisher::StatPublisher(std::string ip, int port) :ip(ip), port(port) {
  statdb = new TCPTransmitter(ip.c_str(), port);
}

///
void StatPublisher::publish(std::shared_ptr<Detector> detector) {
  char buffer[bufferSize];
  int unixtime = (int)time(NULL);

  if (statdb->isValidSocket()) {
    for (int i = 1; i <= detector->statsize(); i++) {
      int len =
          sprintf(buffer, "%s %" PRIi64 " %d\n", detector->statname(i).c_str(),
                  detector->statvalue(i), unixtime);
      statdb->senddata(buffer, len);
    }
  } else {
    handleReconnect();
  }
}

///
void StatPublisher::reconnectHelper() {
    LOG(Sev::Warning, "Carbon/Graphite reconnect attempt {}/{}", retries + 1, maxReconnectAttempts);
    delete statdb;
    statdb = new TCPTransmitter(ip.c_str(), port);
    if (statdb->isValidSocket()) {
      LOG(Sev::Info, "Carbon/Graphite connection re-established");
      retries = 0;
    } else {
      if (retries + 1 == maxReconnectAttempts) {
        LOG(Sev::Error, "Unable to restore Carbon/Graphite connection");
      }
    }
    reconnect.now();
    retries++;
}

///
void StatPublisher::handleReconnect() {
  if (retries == 0) { // Not started yet
    reconnectHelper();
  } else {
    if ((reconnect.timeus() < reconnectDelayUS) || (retries >= maxReconnectAttempts)) {
      return;
    }
    reconnectHelper();
  }
}
