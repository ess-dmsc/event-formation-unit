/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Implements a command server
 */

#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <common/Trace.h>
#include <efu/Server.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

void Server::server_close() {
  XTRACE(MAIN, ALW, "Closing socket %d\n", sock_client);
  close(sock_client);
  sock_client = -1;
}

/** @brief Setup socket parameters
 */
void Server::server_open() {
  XTRACE(INIT, ALW, "Server::open() called on port %d\n", port_);

  struct sockaddr_in socket_address;
  int ret;
  int option=1; // any nonzero value will do

  sock_server = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock_server >= 0);

  ret = setsockopt(sock_server, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  assert(ret >= 0);

  ret = ioctl(sock_server, FIONBIO, &option);
  assert(ret >= 0);

  std::fill_n((char*)&socket_address, sizeof(socket_address), 0);
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
  socket_address.sin_port = htons(port_);

  ret = bind(sock_server, (struct sockaddr *)&socket_address, sizeof(socket_address));
  assert(ret >= 0);

  ret = listen(sock_server, 32); // backlog == 32
  assert(ret >= 0);

  FD_SET(sock_server, &fd_master);
}

/** @brief Called in main program loop
 */
void Server::server_poll() {

  // Prepare working file desc.
  static_assert(sizeof(fd_master) == sizeof(fd_working), "fd_ struct mismatch");
  memcpy(&fd_working, &fd_master, sizeof(fd_master));
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 1000;

  auto max_socket = std::max(sock_server, sock_client) + 1;

  auto ready = select(max_socket, &fd_working, NULL, NULL, &timeout);
  if (ready < 0) {
    return;
  }

  // Server activity
  if (ready > 0 && FD_ISSET(sock_server, &fd_working)) {
    XTRACE(MAIN, ALW, "Incoming connection\n");
    if (sock_client < 0) {
      sock_client = accept(sock_server, NULL, NULL);
      if (sock_client < 0 && errno != EWOULDBLOCK) {
        assert(1 == 0);
      }
      FD_SET(sock_client, &fd_master);
      XTRACE(MAIN, ALW, "sock_client: %d, ready: %d\n", sock_client, ready);
      ready--;
    }
  }

  // Client activity
  if (ready > 0 && FD_ISSET(sock_client, &fd_working)) {
    auto bytes = recv(sock_client, input.data + input.bytes, 9000 - input.bytes, 0);

    if ((bytes < 0) && (errno != EWOULDBLOCK || errno != EAGAIN)) {
      XTRACE(MAIN, ALW, "recv() failed, errno: %d\n", errno);
      perror("recv() failed");
      server_close();
      return;
    }
    if (bytes == 0) {
      XTRACE(MAIN, ALW, "peer closed socket\n");
      server_close();
      return;
    }
    XTRACE(MAIN, ALW, "Received %ld bytes on socket %d\n", bytes, sock_client);
    input.bytes += bytes;
    assert(input.bytes <= 9000);
  }
}
