// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file contains the declaration of the StatPublisher class for
/// transmitting time series metrics to a Graphite/Carbon server over TCP
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/system/Socket.h>
#include <common/Statistics.h>
#include <common/time/Timer.h>
#include <string>

class StatPublisher {
public:
  /// \brief Connect to a Carbon/Graphite server by ip address/hostname and tcp
  /// port
  StatPublisher(std::string IP, int Port);

  /// \brief Send detector metrics to Carbon/Graphite server given additional
  /// stats
  void publish(std::shared_ptr<Detector> DetectorPtr, Statistics &OtherStats);

private:
  /// \brief called when senddata() fails
  void handleReconnect();

  /// \brief
  void reconnectHelper();

  /// Connection variable
  std::unique_ptr<TCPTransmitter> StatDb;

  /// \brief ip address of the stat database server (dotted quad: x.y.z.a)
  std::string IpAddress{""};

  /// \brief TCP port number of database server
  uint16_t TCPPort{0};

  /// Reconnect variables
  /// \brief the number of connection attempts
  unsigned int Retries{1};

  /// \brief timer for determining quiet period
  Timer ReconnectTime;

  /// \brief log an error messages after this many attempts
  /// multiply with reconnectDelayUs to determine when this occurs
  const uint64_t MaxReconnectAttempts{240};

  /// \brief delay in us between reconnection attempts
  const uint64_t ReconnectDelayUS{30'000'000};
};
