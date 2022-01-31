// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Command server implementation
///
//===----------------------------------------------------------------------===//

//#include <algorithm>
#include <arpa/inet.h>
#include <cinttypes>
#include <common/debug/Log.h>
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

Server::Server(int port, Parser &parse) : ServerPort(port), CommandParser(parse) {
  std::fill(ClientFd.begin(), ClientFd.end(), -1);

  Timeout.tv_sec = 0;  /// Timeout for select()
  Timeout.tv_usec = 1000;

  serverOpen();
}

Server::~Server() {
  for (auto &client : ClientFd) {
    close(client);
  }
  close(ServerFd);
}

/// \brief Setup socket parameters
void Server::serverOpen() {
  LOG(IPC, Sev::Info, "Server::serverOpen() called on port {}", ServerPort);

  struct sockaddr_in socket_address;
  int __attribute__((unused)) ret;

  ServerFd = socket(AF_INET, SOCK_STREAM, 0);
  if (ServerFd < 0) {
    LOG(IPC, Sev::Error, "Unable to create tcp socket");
    throw std::runtime_error("socket() failed");
  }

  ret = setsockopt(ServerFd, SOL_SOCKET, SO_REUSEADDR, &SocketOptionOn, sizeof(SocketOptionOn));
  if (ret < 0) {
    LOG(IPC, Sev::Error, "setsockopt() failed");
    throw std::runtime_error("setsockopt() failed");
  }

  std::fill_n((char *)&socket_address, sizeof(socket_address), 0);
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
  socket_address.sin_port = htons(ServerPort);

  ret = bind(ServerFd, (struct sockaddr *)&socket_address, sizeof(socket_address));
  if (ret < 0) {
    LOG(IPC, Sev::Error, "tcp port {} is already in use", ServerPort);
    throw std::runtime_error("tcp port already in use");
  }

  ret = listen(ServerFd, SERVER_MAX_BACKLOG);
  if (ret < 0) {
    LOG(IPC, Sev::Error, "listen() failed");
    throw std::runtime_error("listen() failed");
  }
}

void Server::serverClose(int socket) {
  LOG(IPC, Sev::Debug, "Closing socket fd {}", socket);

  auto client = std::find(ClientFd.begin(), ClientFd.end(), socket);
  if (client == ClientFd.end()) {
    LOG(IPC, Sev::Error, "internal error socket {} not active but attempted closed", socket);
    throw std::runtime_error("serverClose() internal error");
  }
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
      LOG(IPC, Sev::Debug, "Accept new connection, socket {}", newsock);
      *freefd = newsock;
      if (*freefd < 0 && errno != EWOULDBLOCK) {
        LOG(IPC, Sev::Error, "serverPoll() - internal error");
        throw std::runtime_error("serverPoll() - internal error");
      }
      LOG(IPC, Sev::Debug, "New cilent socket: {}", *freefd);
      #ifdef SYSTEM_NAME_DARWIN
        LOG(IPC, Sev::Debug, "setsockopt() - MacOS specific");
        int on = 1;
        int ret = setsockopt(*freefd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
        if (ret != 0) {
            LOG(IPC, Sev::Warning, "Cannot set SO_NOSIGPIPE for socket");
            perror("setsockopt():");
            throw std::runtime_error("setsockopt() failed");
        }
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
        LOG(IPC, Sev::Warning, "recv() failed (unclean close from peer?), errno: {}", errno);
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
      TotalBytesReceived += bytes;
      if (IBuffer.bytes > SERVER_BUFFER_SIZE) {
        LOG(IPC, Sev::Error, "recv() datasize mismatch");
        throw std::runtime_error("recv() datasize mismatch");
      }

      // Parse and generate reply
      if (CommandParser.parse((char *)IBuffer.buffer, IBuffer.bytes, (char *)OBuffer.buffer,
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

int Server::getNumClients() {
  return std::count_if(ClientFd.begin(), ClientFd.end(),
    [](int fd){return fd != -1;});
}
