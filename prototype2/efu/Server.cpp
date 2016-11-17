/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Implements a command server
 */

#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <common/Trace.h>
#include <efu/Server.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_INF

void Server::server_close(int socketfd) {
  XTRACE(IPC, DEB, "Closing socket fd %d\n", sock_client);
  close(socketfd);
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


int Server::server_send(int socketfd) {
  XTRACE(IPC, DEB, "server_send() - %d bytes\n", output.bytes);
  if (send(socketfd, output.buffer, output.bytes, 0) < 0) {
    XTRACE(IPC, WAR, "Error sending command reply\n");
    return -1;
  }
  output.bytes = 0;
  output.data = output.buffer;
  return 0;
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
    auto bytes = recv(sock_client, input.data + input.bytes, SERVER_BUFFER_SIZE - input.bytes, 0);

    if ((bytes < 0) && (errno != EWOULDBLOCK || errno != EAGAIN)) {
      XTRACE(IPC, WAR, "recv() failed, errno: %d\n", errno);
      perror("recv() failed");
      server_close(sock_client);
      return;
    }
    if (bytes == 0) {
      XTRACE(IPC, INF, "Peer closed socket\n");
      server_close(sock_client);
      return;
    }
    XTRACE(IPC, INF, "Received %ld bytes on socket %d\n", bytes, sock_client);
    input.bytes += bytes;

    auto min = std::min(input.bytes, SERVER_BUFFER_SIZE - 1U);
    input.buffer[min] = '\0';
    XTRACE(IPC, DEB, "buffer[] = %s", input.buffer);

    assert(input.bytes <= SERVER_BUFFER_SIZE);
    XTRACE(IPC, DEB, "input.bytes: %d\n", input.bytes);

    // Parse and generate reply
    if (server_parse() < 0) {
      XTRACE(IPC, WAR, "Parse error (unknown command?)\n");
      output.bytes = snprintf((char*)output.buffer, SERVER_BUFFER_SIZE, "Unknown command\n");
    }

    input.bytes = 0;
    input.data = input.buffer;
    if (server_send(sock_client) < 0) {
      XTRACE(IPC, WAR, "server_send() failed\n");
      server_close(sock_client);
      return;
    }
    ready--;
  }
}

int Server::server_parse() {
  auto max = std::max(input.bytes, SERVER_BUFFER_SIZE);
  assert(max > 1);
  if (input.buffer[max - 1] != '\0') {
    XTRACE(IPC, DEB, "Array is NOT null terminated!\n");
    input.buffer[max -1] = '\0';
  }
  if (input.buffer[max - 2] == '\n') {
    XTRACE(IPC, DEB, "Array conatains newline\n");
    input.buffer[max -2] = '\0';
  }

  std::vector<std::string> tokens;
  char * chars_array = strtok((char*)input.buffer, "\n ");
  while(chars_array) {
    std::string token(chars_array);
    tokens.push_back(token);
    chars_array = strtok(NULL, "\n ");
  }
  XTRACE(IPC, DEB, "Tokens in command: %d\n", (int)tokens.size());
  for (auto token : tokens) {
    XTRACE(IPC, DEB, "Token: %s\n", token.c_str());
  }

  if ((int)tokens.size() < 1)
    return -1;

  /** @todo This is really ugly, consider using another approach later */
  if (tokens.at(0).compare(std::string("STAT_INPUT")) == 0) {
    XTRACE(IPC, INF, "STAT_INPUT\n");
    output.bytes = snprintf((char *)output.buffer, SERVER_BUFFER_SIZE,
         "STAT_INPUT %" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 "\n",
        opts.stat.i.rx_packets, opts.stat.i.rx_bytes,
        opts.stat.i.fifo_push_errors, opts.stat.i.fifo_free);

  } else if (tokens.at(0).compare(std::string("STAT_PROCESSING")) == 0) {
    XTRACE(IPC, INF, "STAT_PROCESSING\n");
    output.bytes = snprintf((char *)output.buffer, SERVER_BUFFER_SIZE,
         "STAT_PROCESSING %" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 \
         ", %" PRIu64 ", %" PRIu64 "\n",
        opts.stat.p.rx_events, opts.stat.p.rx_error_bytes, opts.stat.p.rx_discards,
        opts.stat.p.idle,
        opts.stat.p.fifo_push_errors, opts.stat.p.fifo_free);

  } else if (tokens.at(0).compare(std::string("STAT_RESET")) == 0) {
    XTRACE(IPC, INF, "STAT_RESET\n");
    opts.stat.clear();
    output.bytes = snprintf((char *)output.buffer, SERVER_BUFFER_SIZE, "<OK>\n");

  } else if (tokens.at(0).compare(std::string("STAT_MASK")) == 0) {
    if ((int)tokens.size() != 2) {
      XTRACE(IPC, INF, "STAT_MASK wrong number of argument\n");
      return -1;
    }
    unsigned int mask = (unsigned int)std::stoul(tokens.at(1), nullptr, 0);
    XTRACE(IPC, INF, "STAT_MASK 0x%08x\n", mask);
    opts.stat.set_mask(mask);
    output.bytes = snprintf((char *)output.buffer, SERVER_BUFFER_SIZE, "<OK>\n");

  } else {
    return -1;
  }
  return 0;
}
