/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Implements a command server. The server supports multiple
 * clients, and is of type request-response. A client request is
 * handled synchronously to completion before servring next request.
 */

#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <common/EFUArgs.h>
#include <sys/select.h>
#include <sys/types.h>

/** @todo make this work with public static unsigned int */
#define SERVER_BUFFER_SIZE 9000U
#define SERVER_MAX_CLIENTS 16

static_assert(SERVER_MAX_CLIENTS <= FD_SETSIZE, "Too many clients");

class Server {
public:
  /** @brief Server for program control and stats
 *  @param port tcp port
 *  @param args - needed to access Stat.h counters
 */
  Server(int port, Parser &parse);

  /** @brief Setup socket parameters
   */
  void server_open();

  /** @brief Teardown socket
   *  @param socketfd socket file descriptor
   */
  void server_close(int socketfd);

  /** @brief Called in main program loop
   */
  void server_poll();

  /** @brief Send reply to Client
   *  @param socketfd socket file descriptor
   */
  int server_send(int socketfd);

private:
  typedef struct {
    uint8_t buffer[SERVER_BUFFER_SIZE + 1];
    uint8_t *data;
    uint32_t bytes;
  } socket_buffer_t;

  socket_buffer_t input;
  socket_buffer_t output;

  int port_{0};
  int serverfd{-1};
  std::array<int, SERVER_MAX_CLIENTS> clientfd;
  fd_set fd_master, fd_working;

  Parser &parser;
};
