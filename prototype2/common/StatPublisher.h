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
#include <common/NewStats.h>
#include <libs/include/Socket.h>
#include <libs/include/Timer.h>
#include <string>

class StatPublisher {
public:
  /// \brief Connect to a Carbon/Graphite server bu ip address and tcp port
  StatPublisher(std::string ip, int port);

  /// \brief Send detector metrics to Carbon/Graphite server
  void publish(std::shared_ptr<Detector> detector);

  /// \brief Send detector metrics to Carbon/Graphite server given additional stats
  void publish(std::shared_ptr<Detector> detector, NewStats & otherstats);


private:
  static const int BufferSize = 1000;
  char Buffer[BufferSize];

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
  const uint64_t ReconnectDelayUS{30 * 1000 * 1000};
};
