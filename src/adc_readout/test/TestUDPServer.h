/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Code for testing the UDP interface to the ADC detector module.
///
//===----------------------------------------------------------------------===//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <asio.hpp>
#pragma GCC diagnostic pop

#include <asio/steady_timer.hpp>
#include <atomic>
#include <thread>

typedef std::shared_ptr<asio::ip::udp::socket> SocketPtr;

std::uint16_t GetPortNumber();

class TestUDPServer {
public:
  TestUDPServer(std::uint16_t SrcPort, std::uint16_t DstPort, int PacketSize);
  TestUDPServer(std::uint16_t SrcPort, std::uint16_t DstPort,
                std::uint8_t *DataPtr, size_t DataLength);
  // \brief Call before TestUDPServer::startPacketPtransmission() if it is to be
  // called.
  void setConfigPacket(std::uint8_t *DataPtr, size_t DataLength);
  void startPacketTransmission(int TotalPackets, int PacketGapNS);
  ~TestUDPServer();

private:
  asio::io_service Service;
  std::uint16_t SourcePort, DestinationPort;
  void threadFunction();
  std::thread AsioThread;
  asio::ip::udp::socket Socket;
  asio::ip::udp::resolver Resolver;

  bool HasConfigBuffer{false};
  int ConfigBufferSize;
  std::unique_ptr<std::uint8_t[]> ConfigBuffer;

  int BufferSize;
  std::unique_ptr<std::uint8_t[]> SendBuffer;
  void handleWrite(const asio::error_code &Err, std::size_t BytesSent);
  void handleResolve(const asio::error_code &Err,
                     asio::ip::udp::resolver::iterator EndpointIter);
  void handleConnect(const asio::error_code &Err,
                     asio::ip::udp::resolver::iterator EndpointIter);
  void handleNewPacket(const asio::error_code &Err);

  std::uint32_t NrOfPackets{1};
  std::uint32_t WaitTimeNSec{100000};

  bool GotError = false;

  asio::steady_timer PacketTimer;
  //  asio::deadline_timer waitTimer;
};
