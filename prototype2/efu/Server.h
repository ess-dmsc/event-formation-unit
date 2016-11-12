/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Implements a command server
 */

#pragma once
#include <algorithm>
#include <sys/types.h>
#include <sys/select.h>

class Server {

public:

  /** @brief Server for program control and stats
   *  @param port tcp port
   */
  Server(int port) : port_(port) {
    FD_ZERO(&fd_master);
    FD_ZERO(&fd_working);
    std::fill_n((char*)&input, sizeof(input), 0);
    input.data = input.buffer;
    std::fill_n((char*)&output, sizeof(output), 0);
    output.data = output.buffer;
    server_open();
  }

  /** @brief Setup socket parameters
   */
  void server_open();

  /** @brief Teardown socket
   */
  void server_close();

  /** @brief Called in main program loop
   */
  void server_poll();

private:
  typedef struct {
    uint8_t buffer[9000];
    uint8_t *data;
    uint32_t bytes;
  } socket_buffer_t;

  socket_buffer_t input;
  socket_buffer_t output;

  int port_{0};
  int sock_server{-1};
  int sock_client{-1};
  fd_set fd_master, fd_working;
};
