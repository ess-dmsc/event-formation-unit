/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief runtime statistics
 */

#pragma once

#include <common/Detector.h>
#include <libs/include/Socket.h>
#include <string>

class StatPublisher {
public:
  /** @todo document */
  StatPublisher(std::string ip, int port);

  /** @todo document */
  void publish(std::shared_ptr<Detector> detector);

private:
  TCPClient *statdb;
};
