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
#include <string>

class StatPublisher {
public:
  /// Connect to a Carbon/Graphite server bu ip address and tcp port
  StatPublisher(std::string ip, int port);

  /// Send detector metrics to Carbon/Graphite server
  void publish(std::shared_ptr<Detector> detector);

private:
  TCPTransmitter *statdb;
};
