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

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

Server::Server(int port, class Parser &parse) : Port(port), Parser(parse) {
  for (auto &client : ClientFd) {
    client = -1;
  }
  assert(ClientFd[0] == -1);
  assert(ClientFd[SERVER_MAX_CLIENTS -1] == -1);

  Timeout.tv_sec = 0;  /// Timeout for select()
  Timeout.tv_usec = 1000;

  serverOpen();
}

/// \brief Setup socket parameters
void Server::serverOpen() {
  LOG(IPC, Sev::Info, "Server::open() called on port {}", Port);

  struct sockaddr_in socket_address;
  int __attribute__((unused)) ret;

  ServerFd = socket(AF_INET, SOCK_STREAM, 0);
  assert(ServerFd >= 0);

  ret = setsockopt(ServerFd, SOL_SOCKET, SO_REUSEADDR, &SocketOptionOn, sizeof(SocketOptionOn));
  assert(ret >= 0);

  std::fill_n((char *)&socket_address, sizeof(socket_address), 0);
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
  socket_address.sin_port = htons(Port);

  ret = bind(ServerFd, (struct sockaddr *)&socket_address, sizeof(socket_address));
  assert(ret >= 0);

  ret = listen(ServerFd, SERVER_MAX_BACKLOG);
  assert(ret >= 0);
}

void Server::serverClose(int socket) {
  LOG(IPC, Sev::Info, "Closing socket fd {}", socket);

  auto client = std::find(ClientFd.begin(), ClientFd.end(), socket);
  assert(client != ClientFd.end());
  *client = -1;

  close(socket);
}

int Server::serverSend(int socketfd) {
  LOG(IPC, Sev::Debug, "server_send() - socket {} - {} bytes", socketfd, OBuffer.bytes);
  if (send(socketfd, OBuffer.buffer, OBuffer.bytes, SEND_FLAGS) < 0) {
    LOG(IPC, Sev::Warning, "Error sending command reply");
    return -1;
  }
  return 0;
}

/// \brief Called in main program loop
void Server::serverPoll() {
  // Prepare working file desc.
  fd_set fds_working;
  FD_ZERO(&fds_working);
  FD_SET(ServerFd, &fds_working);

  int max_sd = ServerFd;
  for (auto sd : ClientFd) {
    if (sd != -1) {
      FD_SET(sd, &fds_working);
      if (sd > max_sd) {
        max_sd = sd;
      }
    }
  }

  if (select(max_sd + 1, &fds_working, NULL, NULL, &Timeout) <= 0) {
    return; // -1 is error 0 is Timeout, carry on
  }

  // Server has activity
  if (FD_ISSET(ServerFd, &fds_working)) {
    auto newsock = accept(ServerFd, NULL, NULL);

    auto freefd = std::find(ClientFd.begin(), ClientFd.end(), -1);
    if (freefd == ClientFd.end()) {
      LOG(IPC, Sev::Warning, "Max clients connected, can't accept()");
      close(newsock);
    } else {
      LOG(IPC, Sev::Info, "Accept new connection, socket {}", newsock);
      *freefd = newsock;
      if (*freefd < 0 && errno != EWOULDBLOCK) {
        assert(1 == 0);
      }
      LOG(IPC, Sev::Info, "New cilent socket: {}", *freefd);
      #ifdef SYSTEM_NAME_DARWIN
        LOG(IPC, Sev::Info, "setsockopt() - MacOS specific");
        int on = 1;
        int ret = setsockopt(*freefd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
        if (ret != 0) {
            LOG(IPC, Sev::Warning, "Cannot set SO_NOSIGPIPE for socket");
            perror("setsockopt():");
        }
        assert(ret == 0);
      #endif
    }
  }

  // Chek if some client has activity
  for (auto sd : ClientFd) {
    if (sd == -1) {
      continue;
    }
    //printf("Checking for activity on socket %d\n", sd);
    if (FD_ISSET(sd, &fds_working)) {
      auto bytes = recv(sd, IBuffer.buffer, SERVER_BUFFER_SIZE, 0);

      if ((bytes < 0) && (errno != EWOULDBLOCK) && (errno != EAGAIN)) {
        LOG(IPC, Sev::Warning, "recv() failed, errno: {}", errno);
        perror("recv() failed");
        serverClose(sd);
        return;
      }
      if (bytes == 0) {
        LOG(IPC, Sev::Debug, "Peer closed socket {}", sd);
        serverClose(sd);
        return;
      }
      LOG(IPC, Sev::Debug, "Received {} bytes on socket {}", bytes, sd);
      IBuffer.bytes = bytes;
      assert(IBuffer.bytes <= SERVER_BUFFER_SIZE);

      // Parse and generate reply
      if (Parser.parse((char *)IBuffer.buffer, IBuffer.bytes, (char *)OBuffer.buffer,
                       &OBuffer.bytes) < 0) {
        LOG(IPC, Sev::Warning, "Parse error");
      }
      if (serverSend(sd) < 0) {
        LOG(IPC, Sev::Warning, "server_send() failed");
        serverClose(sd);
        return;
      }
    }
  }
}
