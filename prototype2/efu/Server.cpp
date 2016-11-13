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

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

void Server::server_close() {
  XTRACE(IPC, DEB, "Closing socket fd %d\n", sock_client);
  close(sock_client);
  sock_client = -1;
}

/** @brief Setup socket parameters
 */
void Server::server_open() {
  XTRACE(IPC, INF, "Server::open() called on port %d\n", port_);

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

  // Server has activity
  if (ready > 0 && FD_ISSET(sock_server, &fd_working)) {
    if (sock_client < 0) {
      XTRACE(IPC, INF, "Incoming connection\n");
      sock_client = accept(sock_server, NULL, NULL);
      if (sock_client < 0 && errno != EWOULDBLOCK) {
        assert(1 == 0);
      }
      FD_SET(sock_client, &fd_master);
      XTRACE(IPC, DEB, "sock_client: %d, ready: %d\n", sock_client, ready);
      ready--;
    }
  }

  // Client has activity
  if (ready > 0 && FD_ISSET(sock_client, &fd_working)) {
    auto bytes = recv(sock_client, input.data + input.bytes, 9000 - input.bytes, 0);

    if ((bytes < 0) && (errno != EWOULDBLOCK || errno != EAGAIN)) {
      XTRACE(IPC, WAR, "recv() failed, errno: %d\n", errno);
      perror("recv() failed");
      server_close();
      return;
    }
    if (bytes == 0) {
      XTRACE(IPC, INF, "Peer closed socket\n");
      server_close();
      return;
    }
    XTRACE(IPC, INF, "Received %ld bytes on socket %d\n", bytes, sock_client);
    input.bytes += bytes;

    auto min = std::min(input.bytes, 9000U - 1U);
    input.buffer[min] = '\0';
    XTRACE(IPC, DEB, "buffer[] = %s", input.buffer);

    assert(input.bytes <= 9000);
    XTRACE(IPC, DEB, "input.bytes: %d\n", input.bytes);

    // Parse and generate reply
    memcpy(output.buffer, input.buffer, input.bytes);
    input.bytes = 0;
    input.data = input.buffer;
    if (server_send() < 0) {
      server_close();
      return;
    }
    ready--;
  }
}


int Server::server_send() {
  if (send(sock_client, output.buffer, output.bytes, 0) < 0) {
    XTRACE(MAIN, WAR, "Error sending command reply\n");
    return -1;
  }
  output.bytes = 0;
  output.data = output.buffer;
  return 0;
}
