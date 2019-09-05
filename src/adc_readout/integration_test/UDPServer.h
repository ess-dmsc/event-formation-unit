/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <asio.hpp>
#pragma GCC diagnostic pop

#include <atomic>
#include <map>
#include <thread>

typedef std::shared_ptr<asio::ip::udp::socket> SocketPtr;
typedef std::unique_ptr<asio::io_service::work> WorkPtr;
typedef std::shared_ptr<std::uint8_t> BufferPtr;

class UDPServer {
public:
  UDPServer(std::uint16_t SrcPort, std::uint16_t DstPort);
  ~UDPServer();

  bool IsOk() const { return ConnectionOk; };
  bool TransmitPacket(const std::uint8_t *DataPtr, const std::uint32_t Size);

private:
  std::atomic_bool ConnectionOk{false};
  std::atomic<std::int64_t> PacketsSent{0};
  asio::io_service Service;
  WorkPtr Work;
  void threadFunction();
  std::thread AsioThread;
  asio::ip::udp::socket Socket;
  asio::ip::udp::resolver Resolver;

  void handleWrite(const asio::error_code &Err, std::size_t, BufferPtr Buffer);
  void handleResolve(const asio::error_code &Err,
                     asio::ip::udp::resolver::iterator EndpointIter);
  void handleConnect(const asio::error_code &Err,
                     asio::ip::udp::resolver::iterator EndpointIter);
  void handlePacketTransmit(BufferPtr Buffer, const std::uint32_t Size);
};
