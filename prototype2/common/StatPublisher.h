/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief runtime statistics
 */

#pragma once

#include <common/EFUStats.h>
#include <libs/include/Socket.h>
#include <string>

class StatPublisher {
public:
  StatPublisher(std::string ip, int port);

  void publish(EFUStats::stat_t *s);

private:
  TCPClient *statdb;
};
