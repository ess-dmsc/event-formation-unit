// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of StatPublisher
///
//===----------------------------------------------------------------------===//
//
#include <common/StatPublisher.h>
#include <common/debug/Log.h>
#include <fmt/format.h>

///
StatPublisher::StatPublisher(const std::string &IP, int Port)
    : IpAddress(IP), TCPPort(Port) {
  if (not SocketImpl::isValidIp(IpAddress)) {
    IpAddress = SocketImpl::getHostByName(IpAddress);
  }
  StatDb.reset(new TCPTransmitter(IpAddress.c_str(), TCPPort));
}

///
void StatPublisher::publish(std::shared_ptr<Detector> DetectorPtr,
                            Statistics &OtherStats) {
  int unixtime = (int)time(NULL);

  if (StatDb->isValidSocket()) {
    for (int i = 1; i <= DetectorPtr->statsize(); i++) {
      auto StatString = fmt::format("{} {} {}\n", DetectorPtr->getStatFullName(i),
                                    DetectorPtr->statvalue(i), unixtime);

      StatDb->senddata(StatString.c_str(), StatString.size());
    }
    for (size_t i = 1; i <= OtherStats.size(); i++) {
      auto StatString = fmt::format("{} {} {}\n", OtherStats.getFullName(i),
                                    OtherStats.getValue(i), unixtime);

      StatDb->senddata(StatString.c_str(), StatString.size());
    }
  } else {
    handleReconnect();
  }
}

///
void StatPublisher::reconnectHelper() {
  LOG(UTILS, Sev::Warning, "Carbon/Graphite reconnect attempt {}", Retries);
  StatDb.reset(new TCPTransmitter(IpAddress.c_str(), TCPPort));
  if (StatDb->isValidSocket()) {
    LOG(UTILS, Sev::Info, "Carbon/Graphite connection re-established");
    Retries = 1;
  } else {
    if (Retries == MaxReconnectAttempts) {
      LOG(UTILS, Sev::Error,
          "Unable to restore Carbon/Graphite connection for {} seconds",
          MaxReconnectAttempts * ReconnectDelayUS / 1000000.0);
    }
  }
  ReconnectTime.reset();
  Retries++;
}

///
void StatPublisher::handleReconnect() {
  if (Retries == 1) { // Not started yet
    reconnectHelper();
  } else {
    if (ReconnectTime.timeUS() < ReconnectDelayUS) {
      return;
    }
    reconnectHelper();
  }
}
