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
#include <signal.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

Server::Server(int port, Parser &parse) : port_(port), parser(parse) {
  for (auto &client : clientfd) {
    client = -1;
  }
  assert(clientfd[0] == -1);
  assert(clientfd[SERVER_MAX_CLIENTS -1] == -1);

  timeout.tv_sec = 0;
  timeout.tv_usec = 1000;

  serverOpen();
}

/// \brief Setup socket parameters
void Server::serverOpen() {
  LOG(Sev::Info, "Server::open() called on port {}", port_);

  struct sockaddr_in socket_address;
  UNUSED int ret;
  int option = 1; // any nonzero value will do

  serverfd = socket(AF_INET, SOCK_STREAM, 0);
  assert(serverfd >= 0);

  ret = setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  assert(ret >= 0);

  // ret = ioctl(serverfd, FIONBIO, &option);
  // assert(ret >= 0);

  std::fill_n((char *)&socket_address, sizeof(socket_address), 0);
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
  socket_address.sin_port = htons(port_);

  ret = bind(serverfd, (struct sockaddr *)&socket_address,
             sizeof(socket_address));
  assert(ret >= 0);

  ret = listen(serverfd, SERVER_MAX_BACKLOG);
  assert(ret >= 0);
}

void Server::serverClose(int socket) {
  LOG(Sev::Debug, "Closing socket fd {}", socket);

  auto client = std::find(clientfd.begin(), clientfd.end(), socket);
  assert(client != clientfd.end());
  *client = -1;

  close(socket);
}



int Server::serverSend(int socketfd) {
  LOG(Sev::Debug, "server_send() - socket {} - {} bytes", socketfd, output.bytes);
  if (send(socketfd, output.buffer, output.bytes, SEND_FLAGS) < 0) {
    LOG(Sev::Warning, "Error sending command reply");
    return -1;
  }
  return 0;
}

/** \brief Called in main program loop
 */
void Server::serverPoll() {
  // Prepare working file desc.
  fd_set fds_working;
  FD_ZERO(&fds_working);
  FD_SET(serverfd, &fds_working);

  int max_sd = serverfd;

  for (auto sd : clientfd) {
    if (sd != -1) {
      FD_SET(sd, &fds_working);
      if (sd > max_sd) {
        max_sd = sd;
      }
    }
  }

  if (select(max_sd + 1, &fds_working, NULL, NULL, &timeout) <= 0) {
    return; // -1 is error 0 is timeout, carry on
  }

  // Server has activity
  if (FD_ISSET(serverfd, &fds_working)) {
    auto newsock = accept(serverfd, NULL, NULL);

    auto freefd = std::find(clientfd.begin(), clientfd.end(), -1);
    if (freefd == clientfd.end()) {
      LOG(Sev::Warning, "Max clients connected, can't accept()");
      close(newsock);
    } else {
      LOG(Sev::Info, "Accept new connection, socket {}", newsock);
      *freefd = newsock;
      if (*freefd < 0 && errno != EWOULDBLOCK) {
        assert(1 == 0);
      }
      LOG(Sev::Debug, "New cilent socket: {}", *freefd);
      #ifdef SYSTEM_NAME_DARWIN
        LOG(Sev::Info, "setsockopt() - MacOS specific");
        int on = 1;
        int ret = setsockopt(*freefd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
        if (ret != 0) {
            LOG(Sev::Warning, "Cannot set SO_NOSIGPIPE for socket");
            perror("setsockopt():");
        }
        assert(ret == 0);
      #endif
    }
  }

  // Chek if some client has activity
  for (auto sd : clientfd) {
    if (sd == -1) {
      break;
    }
    //printf("Checking for activity on socket %d\n", sd);
    if (FD_ISSET(sd, &fds_working)) {
      auto bytes = recv(sd, input.buffer, SERVER_BUFFER_SIZE, 0);

      if ((bytes < 0) && (errno != EWOULDBLOCK) && (errno != EAGAIN)) {
        LOG(Sev::Warning, "recv() failed, errno: {}", errno);
        perror("recv() failed");
        serverClose(sd);
        return;
      }
      if (bytes == 0) {
        LOG(Sev::Info, "Peer closed socket {}", sd);
        serverClose(sd);
        return;
      }
      LOG(Sev::Debug, "Received {} bytes on socket {}", input.bytes, sd);
      input.bytes = bytes;
      assert(input.bytes <= SERVER_BUFFER_SIZE);
      LOG(Sev::Debug, "input.bytes: {}", input.bytes);

      // Parse and generate reply
      if (parser.parse((char *)input.buffer, input.bytes, (char *)output.buffer,
                       &output.bytes) < 0) {
        LOG(Sev::Warning, "Parse error");
      }
      if (serverSend(sd) < 0) {
        LOG(Sev::Warning, "server_send() failed");
        serverClose(sd);
        return;
      }
    }
  }
}
