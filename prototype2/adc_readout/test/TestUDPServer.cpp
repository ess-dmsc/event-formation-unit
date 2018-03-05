/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Code for testing the ADC UDP functionality.
 */

#include <functional>
#include <iostream>
#include "TestUDPServer.h"
#include <ciso646>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
TestUDPServer::TestUDPServer(std::uint16_t SrcPort, std::uint16_t DstPort, int PacketSize) : Service(), Socket(Service, asio::ip::udp::endpoint(asio::ip::udp::v4(), SrcPort)), Resolver(Service), PacketTimer(Service), SourcePort(SrcPort), DestinationPort(DstPort) {
  BufferSize = PacketSize;
  SendBuffer = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[PacketSize]);
}

TestUDPServer::TestUDPServer(std::uint16_t SrcPort, std::uint16_t DstPort, std::uint8_t *DataPtr, size_t DataLength)
: Service(), Socket(Service, asio::ip::udp::endpoint(asio::ip::udp::v4(), SrcPort)), Resolver(Service), PacketTimer(Service), SourcePort(SrcPort), DestinationPort(DstPort) {
  BufferSize = DataLength;
  SendBuffer = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[DataLength]);
  std::memcpy(SendBuffer.get(), DataPtr, DataLength);
}
#pragma GCC diagnostic pop

void TestUDPServer::startPacketTransmission(int TotalPackets, int PacketGapNS) {
  NrOfPackets = TotalPackets;
  WaitTimeNSec = PacketGapNS;
  asio::ip::udp::resolver::query Query(asio::ip::udp::v4(), "localhost", std::to_string(DestinationPort));
  Resolver.async_resolve(Query, std::bind(&TestUDPServer::handleResolve, this, std::placeholders::_1, std::placeholders::_2));
  AsioThread = std::thread(&TestUDPServer::threadFunction, this);
}

TestUDPServer::~TestUDPServer() {
  Service.stop();
  AsioThread.join();
  Socket.close();
}

void TestUDPServer::handleWrite(const asio::error_code &Err, std::size_t __attribute__((unused)) BytesSent) {
  if (Err) {
    GotError = true;
  }
}

void TestUDPServer::threadFunction() {
  Service.run();
}

void TestUDPServer::handleResolve(const asio::error_code __attribute__((unused)) &Err, asio::ip::udp::resolver::iterator EndpointIter) {
  asio::ip::udp::endpoint Endpoint = *EndpointIter;
  Socket.async_connect(Endpoint, std::bind(&TestUDPServer::handleConnect, this, std::placeholders::_1, ++EndpointIter));
}

void TestUDPServer::handleConnect(const asio::error_code &Err,
                                  asio::ip::udp::resolver::iterator EndpointIter) {
  if (!Err) {
    Socket.async_send(asio::buffer(SendBuffer.get(), BufferSize), std::bind(&TestUDPServer::handleWrite, this, std::placeholders::_1, std::placeholders::_2));
    --NrOfPackets;
    PacketTimer.expires_from_now(std::chrono::nanoseconds(WaitTimeNSec));
    PacketTimer.async_wait(std::bind(&TestUDPServer::handleNewPacket, this, std::placeholders::_1));
  }
  else if (EndpointIter != asio::ip::udp::resolver::iterator())
  {
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
    Socket.async_send(asio::buffer(SendBuffer.get(), BufferSize), std::bind(&TestUDPServer::handleWrite, this, std::placeholders::_1, std::placeholders::_2));
    PacketTimer.expires_from_now(std::chrono::nanoseconds(WaitTimeNSec));
    PacketTimer.async_wait(std::bind(&TestUDPServer::handleNewPacket, this, std::placeholders::_1));
  }
}


