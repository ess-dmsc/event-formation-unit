/** Copyright (C) 2018 - 2020 European Spallation Source ERIC */

/** @file
 *
 *  \brief Code for testing the ADC UDP functionality.
 */
// GCOVR_EXCL_START

#include <test/TestUDPServer.h>
#include <ciso646>
#include <cstring>
#include <functional>
#include <iostream>
#include <random>

std::uint16_t GetPortNumber() {
  static std::uint16_t CurrentPortNumber = 0;
  if (0 == CurrentPortNumber) {
    std::random_device Device;
    std::mt19937 Generator(Device());
    std::uniform_int_distribution<std::uint16_t> Distribution(2048, 60000);
    CurrentPortNumber = Distribution(Generator);
  }
  return ++CurrentPortNumber;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
TestUDPServer::TestUDPServer(std::uint16_t SrcPort, std::uint16_t DstPort,
                             int PacketSize)
    : Service(), SourcePort(SrcPort), DestinationPort(DstPort),
      Socket(Service, asio::ip::udp::endpoint(asio::ip::udp::v4(), SourcePort)),
      Resolver(Service), BufferSize(PacketSize),
      SendBuffer(std::make_unique<std::uint8_t[]>(PacketSize)),
      PacketTimer(Service) {}

TestUDPServer::TestUDPServer(std::uint16_t SrcPort, std::uint16_t DstPort,
                             std::uint8_t *DataPtr, size_t DataLength)
    : Service(), SourcePort(SrcPort), DestinationPort(DstPort),
      Socket(Service, asio::ip::udp::endpoint(asio::ip::udp::v4(), SrcPort)),
      Resolver(Service), BufferSize(DataLength),
      SendBuffer(std::make_unique<std::uint8_t[]>(DataLength)),
      PacketTimer(Service) {
  std::memcpy(SendBuffer.get(), DataPtr, DataLength);
}
#pragma GCC diagnostic pop

void TestUDPServer::setConfigPacket(std::uint8_t *DataPtr, size_t DataLength) {
  ConfigBuffer = std::make_unique<std::uint8_t[]>(DataLength);
  ConfigBufferSize = DataLength;
  std::memcpy(ConfigBuffer.get(), DataPtr, DataLength);
  HasConfigBuffer = true;
}

void TestUDPServer::startPacketTransmission(int TotalPackets, int PacketGapNS) {
  NrOfPackets = TotalPackets;
  WaitTimeNSec = PacketGapNS;
  asio::ip::udp::resolver::query Query(asio::ip::udp::v4(), "localhost",
                                       std::to_string(DestinationPort));
  Resolver.async_resolve(Query, std::bind(&TestUDPServer::handleResolve, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2));
  AsioThread = std::thread(&TestUDPServer::threadFunction, this);
}

TestUDPServer::~TestUDPServer() {
  Service.stop();
  if (AsioThread.joinable()) {
    AsioThread.join();
  }
  Socket.close();
}

void TestUDPServer::handleWrite(const asio::error_code &Err,
                                std::size_t __attribute__((unused)) BytesSent) {
  if (Err) {
    GotError = true;
  }
}

void TestUDPServer::threadFunction() { Service.run(); }

void TestUDPServer::handleResolve(
    const asio::error_code __attribute__((unused)) & Err,
    asio::ip::udp::resolver::iterator EndpointIter) {
  asio::ip::udp::endpoint Endpoint = *EndpointIter;
  Socket.async_connect(Endpoint,
                       std::bind(&TestUDPServer::handleConnect, this,
                                 std::placeholders::_1, ++EndpointIter));
}

void TestUDPServer::handleConnect(
    const asio::error_code &Err,
    asio::ip::udp::resolver::iterator EndpointIter) {
  if (!Err) {
    auto UsedBuffer = asio::buffer(SendBuffer.get(), BufferSize);
    if (HasConfigBuffer) {
      UsedBuffer = asio::buffer(ConfigBuffer.get(), ConfigBufferSize);
      ++NrOfPackets;
    }
    Socket.async_send(UsedBuffer,
                      std::bind(&TestUDPServer::handleWrite, this,
                                std::placeholders::_1, std::placeholders::_2));
    --NrOfPackets;
    PacketTimer.expires_from_now(std::chrono::nanoseconds(WaitTimeNSec));
    PacketTimer.async_wait(std::bind(&TestUDPServer::handleNewPacket, this,
                                     std::placeholders::_1));
  } else if (EndpointIter != asio::ip::udp::resolver::iterator()) {
    Socket.close();
    asio::ip::udp::endpoint Endpoint = *EndpointIter;
    Socket.async_connect(Endpoint,
                         std::bind(&TestUDPServer::handleConnect, this,
                                   std::placeholders::_1, ++EndpointIter));
  }
}

void TestUDPServer::handleNewPacket(const asio::error_code &Err) {
  if (!Err and NrOfPackets > 0 and not GotError) {
    --NrOfPackets;
    Socket.async_send(asio::buffer(SendBuffer.get(), BufferSize),
                      std::bind(&TestUDPServer::handleWrite, this,
                                std::placeholders::_1, std::placeholders::_2));
    PacketTimer.expires_from_now(std::chrono::nanoseconds(WaitTimeNSec));
    PacketTimer.async_wait(std::bind(&TestUDPServer::handleNewPacket, this,
                                     std::placeholders::_1));
  }
}
// GCOVR_EXCL_STOP
