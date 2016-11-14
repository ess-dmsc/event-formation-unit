/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Implements a command server
 */

#pragma once
#include <algorithm>
#include <common/EFUArgs.h>
#include <sys/types.h>
#include <sys/select.h>

/** @todo make this work with public static unsigned int */
#define SERVER_BUFFER_SIZE 9000U

class Server {
public:
    /** @brief Server for program control and stats
   *  @param port tcp port
   */
  Server(int port, EFUArgs & args)
        : port_(port)
        , opts(args) {
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

  /** @brief Send reply to Client
   */
  int server_send();

  /** @brief Parse request and generate reply
   */
  int server_parse();

private:
  typedef struct {
    uint8_t buffer[SERVER_BUFFER_SIZE + 1];
    uint8_t *data;
    uint32_t bytes;
  } socket_buffer_t;

  socket_buffer_t input;
  socket_buffer_t output;

  int port_{0};
  int sock_server{-1};
  int sock_client{-1};
  fd_set fd_master, fd_working;

  EFUArgs & opts; /** @todo dont like this */
};
