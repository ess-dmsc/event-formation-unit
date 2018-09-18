/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of StatPublisher
///
//===----------------------------------------------------------------------===//
//
#include <common/StatPublisher.h>
#include <common/Log.h>

///
StatPublisher::StatPublisher(std::string ip, int port) :IpAddress(ip), TCPPort(port) {
  StatDb.reset(new TCPTransmitter(IpAddress.c_str(), TCPPort));
}

///
void StatPublisher::publish(std::shared_ptr<Detector> detector) {
  int unixtime = (int)time(NULL);

  if (StatDb->isValidSocket()) {
    for (int i = 1; i <= detector->statsize(); i++) {
      int len = snprintf(Buffer, BufferSize, "%s %" PRIu64 " %d\n", detector->statname(i).c_str(),
                  detector->statvalue(i), unixtime);
      if (len > 0) {
        StatDb->senddata(Buffer, len);
      }
    }
  } else {
    handleReconnect();
  }
}

///
void StatPublisher::reconnectHelper() {
    LOG(Sev::Warning, "Carbon/Graphite reconnect attempt {}", Retries);
    StatDb.reset(new TCPTransmitter(IpAddress.c_str(), TCPPort));
    if (StatDb->isValidSocket()) {
      LOG(Sev::Info, "Carbon/Graphite connection re-established");
      Retries = 1;
    } else {
      if (Retries == MaxReconnectAttempts) {
        LOG(Sev::Error, "Unable to restore Carbon/Graphite connection for {} seconds", MaxReconnectAttempts * ReconnectDelayUS / 1000000.0);
      }
    }
    ReconnectTime.now();
    Retries++;
}

///
void StatPublisher::handleReconnect() {
  if (Retries == 1) { // Not started yet
    reconnectHelper();
  } else {
    if ( ReconnectTime.timeus() < ReconnectDelayUS ) {
      return;
    }
    reconnectHelper();
  }
}
