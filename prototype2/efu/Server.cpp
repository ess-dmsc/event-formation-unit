/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  \brief Implements a command server
 */

//#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <common/Log.h>
#include <cstdio>
#include <cstring>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#define UNUSED __attribute__((unused))

Server::Server(int port, Parser &parse) : port_(port), parser(parse) {

  for (auto &client : clientfd) {
    client = -1;
  }
  assert(clientfd[0] == -1);
  FD_ZERO(&fd_master);
  FD_ZERO(&fd_working);
  std::fill_n((char *)&input, sizeof(input), 0);
  input.data = input.buffer;
  std::fill_n((char *)&output, sizeof(output), 0);
  output.data = output.buffer;
  server_open();
}

void Server::server_close(int socket) {
  LOG(Sev::Debug, "Closing socket fd {}", socket);
  close(socket);
  auto client = std::find(clientfd.begin(), clientfd.end(), socket);
  assert(client != clientfd.end());
  *client = -1;
}

/** \brief Setup socket parameters
 */
void Server::server_open() {
  LOG(Sev::Info, "Server::open() called on port {}", port_);

  struct sockaddr_in socket_address;
  UNUSED int ret;
  int option = 1; // any nonzero value will do

  serverfd = socket(AF_INET, SOCK_STREAM, 0);
  assert(serverfd >= 0);

  ret = setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  assert(ret >= 0);

  ret = ioctl(serverfd, FIONBIO, &option);
  assert(ret >= 0);

  std::fill_n((char *)&socket_address, sizeof(socket_address), 0);
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
  socket_address.sin_port = htons(port_);

  ret = bind(serverfd, (struct sockaddr *)&socket_address,
             sizeof(socket_address));
  assert(ret >= 0);

  ret = listen(serverfd, SERVER_MAX_CLIENTS);
  assert(ret >= 0);

  FD_SET(serverfd, &fd_master);
}

int Server::server_send(int socketfd) {
  LOG(Sev::Debug, "server_send() - {} bytes", output.bytes);
  if (send(socketfd, output.buffer, output.bytes, 0) < 0) {
    LOG(Sev::Warning, "Error sending command reply");
    return -1;
  }
  output.bytes = 0;
  output.data = output.buffer;
  return 0;
}

/** \brief Called in main program loop
 */
void Server::server_poll() {

  // Prepare working file desc.
  static_assert(sizeof(fd_master) == sizeof(fd_working), "fd_ struct mismatch");
  memcpy(&fd_working, &fd_master, sizeof(fd_master));
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 1000;

  auto max_client = std::max_element(clientfd.begin(), clientfd.end());
  auto max_socket = std::max(serverfd, *max_client) + 1;

  auto ready = select(max_socket, &fd_working, NULL, NULL, &timeout);
  if (ready < 0) {
    return;
  }

  // Server has activity
  if (ready > 0 && FD_ISSET(serverfd, &fd_working)) {
    auto freefd = std::find(clientfd.begin(), clientfd.end(), -1);
    if (freefd == clientfd.end()) {
      LOG(Sev::Warning, "Max clients connected, can't accept()");
      auto tmpsock = accept(serverfd, NULL, NULL);
      close(tmpsock);
    } else {
      LOG(Sev::Info, "Accept new connection");
      *freefd = accept(serverfd, NULL, NULL);
      if (*freefd < 0 && errno != EWOULDBLOCK) {
        assert(1 == 0);
      }
      FD_SET(*freefd, &fd_master);
      LOG(Sev::Debug, "New clent socket: {}, ready: {}", *freefd, ready);
    }
    ready--;
  }

  // Chek if some client has activity
  for (auto cli : clientfd) {
    if (ready > 0 && FD_ISSET(cli, &fd_working)) {
      auto bytes = recv(cli, input.data + input.bytes,
                        SERVER_BUFFER_SIZE - input.bytes, 0);

      if ((bytes < 0) && (errno != EWOULDBLOCK || errno != EAGAIN)) {
        LOG(Sev::Warning, "recv() failed, errno: {}", errno);
        perror("recv() failed");
        server_close(cli);
        return;
      }
      if (bytes == 0) {
        LOG(Sev::Info, "Peer closed socket {}", cli);
        server_close(cli);
        return;
      }
      LOG(Sev::Debug, "Received {} bytes on socket {}", bytes, cli);
      input.bytes += bytes;

      assert(input.bytes <= SERVER_BUFFER_SIZE);
      LOG(Sev::Debug, "input.bytes: {}", input.bytes);

      // Parse and generate reply
      if (parser.parse((char *)input.buffer, input.bytes, (char *)output.buffer,
                       &output.bytes) < 0) {
        LOG(Sev::Warning, "Parse error");
      }
      input.bytes = 0;
      input.data = input.buffer;
      if (server_send(cli) < 0) {
        LOG(Sev::Warning, "server_send() failed");
        server_close(cli);
        return;
      }
      ready--;
    }
  }
}
