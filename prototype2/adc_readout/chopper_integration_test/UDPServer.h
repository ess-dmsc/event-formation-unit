//
// Created by Jonas Nilsson on 2017-11-08.
//

#pragma once

#define ASIO_STANDALONE

#include <asio.hpp>
#include <thread>
#include <atomic>
#include <map>

typedef std::shared_ptr<asio::ip::udp::socket> SocketPtr;
typedef std::unique_ptr<asio::io_service::work> WorkPtr;
typedef std::shared_ptr<std::uint8_t> BufferPtr;

class UDPServer {
public:
  UDPServer(std::uint16_t SrcPort, std::uint16_t DstPort);
  ~UDPServer();
  
  bool IsOk() {return ConnectionOk;};
  bool TransmitPacket(const std::uint8_t *DataPtr, const std::uint32_t Size);
private:
  std::atomic_bool ConnectionOk = {false};
  std::atomic_uint64_t PacketsSent = {0};
  asio::io_service Service;
  WorkPtr Work;
  void threadFunction();
  std::thread AsioThread;
  asio::ip::udp::socket Socket;
  asio::ip::udp::resolver Resolver;
  
  void handleWrite(const asio::error_code &err, std::size_t BytesSent, BufferPtr Buffer);
  void handleResolve(const asio::error_code &Err, asio::ip::udp::resolver::iterator EndpointIter);
  void handleConnect(const asio::error_code &Err, asio::ip::udp::resolver::iterator EndpointIter);
  void handlePacketTransmit(BufferPtr Buffer, const std::uint32_t Size);
};
