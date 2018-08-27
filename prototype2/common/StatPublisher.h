/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the StatPublisher class for transmitting
/// time series metrics to a Graphite/Carbon server over TCP
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Detector.h>
#include <libs/include/Socket.h>
#include <libs/include/Timer.h>
#include <string>

class StatPublisher {
public:
  /// \brief Connect to a Carbon/Graphite server bu ip address and tcp port
  StatPublisher(std::string ip, int port);

  /// \brief Send detector metrics to Carbon/Graphite server
  void publish(std::shared_ptr<Detector> detector);

  /// \brief called when senddata() fails
  void handleReconnect();

  /// \brief
  void reconnectHelper();

private:
  /// Connection variable
  TCPTransmitter *statdb;
  std::string ip{""};
  uint16_t port{0};

  /// Reconnect variables
  int retries{0};
  Timer reconnect;

  const int maxReconnectAttempts{240}; /// equivalent to a couple of hours
  const uint64_t reconnectDelayUS{30000000}; /// 30s
};
