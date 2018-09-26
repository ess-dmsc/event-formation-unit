/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// Implements a command server. The server supports multiple
/// clients, and is of type request-response. A client request is
/// handled synchronously to completion before servring next request.
///
//===----------------------------------------------------------------------===//

#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <common/EFUArgs.h>
#include <efu/Parser.h>
#include <sys/select.h>
#include <sys/types.h>

/// \brief Use MSG_SIGNAL on Linuxes
#ifdef MSG_NOSIGNAL
#define SEND_FLAGS MSG_NOSIGNAL
#else
#define SEND_FLAGS 0
#endif

/** \todo make this work with public static unsigned int */
#define SERVER_BUFFER_SIZE 9000U
#define SERVER_MAX_CLIENTS 16
#define SERVER_MAX_BACKLOG 3

static_assert(SERVER_MAX_CLIENTS <= FD_SETSIZE, "Too many clients");

class Server {
public:
  /// \brief Server for program control and stats
  /// \param port tcp port
  /// \param args - needed to access Stat.h counters
  Server(int port, Parser &parse);

  /// \brief Setup socket parameters
  void serverOpen();

  /// \brief Teardown socket
  /// \param socketfd socket file descriptor
  void serverClose(int socketfd);

  /// \brief Called in main program loop
  void serverPoll();

  /// \brief Send reply to Client
  /// \param socketfd socket file descriptor
  int serverSend(int socketfd);

private:
  struct {
    uint8_t buffer[SERVER_BUFFER_SIZE + 1];
    uint32_t bytes;
  } IBuffer, OBuffer; /// receive and transmit buffers

  //SocketBuffer IBuffer; /// receive buffer
  //SocketBuffer OBuffer; /// transmit buffer

  int Port{0}; /// server tcp port
  int ServerFd{-1}; /// server file descriptor
  std::array<int, SERVER_MAX_CLIENTS> ClientFd;

  struct timeval Timeout; /// to set select() timeout

  int SocketOptionOn{1}; // any nonzero value will do

  Parser &CommandParser;
};
