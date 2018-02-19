//
// Created by Jonas Nilsson on 2017-11-08.
//

#include <functional>
#include <iostream>
#include "TestUDPServer.h"
#include <ciso646>

TestUDPServer::TestUDPServer(std::uint16_t SrcPort, std::uint16_t DstPort, std::uint32_t Packets, std::uint32_t WaitTimeNS)
    : Service(), Socket(Service, asio::ip::udp::endpoint(asio::ip::udp::v4(), SrcPort)), Resolver(Service), NrOfPackets(Packets), WaitTimeNSec(WaitTimeNS), PacketTimer(Service) {

      asio::ip::udp::resolver::query Query(asio::ip::udp::v4(), "localhost", std::to_string(DstPort));
  Resolver.async_resolve(Query, std::bind(&TestUDPServer::handleResolve, this, std::placeholders::_1, std::placeholders::_2));
//      Resolver.async_resolve(Query, [this](asio::error_code Err, asio::ip::udp::resolver::iterator Iter){
//        TestUDPServer::handleResolve(Err, Iter);
//      });
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
//  Socket.async_connect(Endpoint, [this](asio::error_code &Err, asio::ip::udp::resolver::iterator EndpointIter){TestUDPServer::handleConnect(Err, ++EndpointIter);});
}

void TestUDPServer::handleConnect(const asio::error_code &Err,
                                  asio::ip::udp::resolver::iterator EndpointIter) {
  if (!Err) {
    Socket.async_send(asio::buffer(SendBuffer, BufferSize), std::bind(&TestUDPServer::handleWrite, this, std::placeholders::_1, std::placeholders::_2));
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
    Socket.async_send(asio::buffer(SendBuffer, BufferSize), std::bind(&TestUDPServer::handleWrite, this, std::placeholders::_1, std::placeholders::_2));
    PacketTimer.expires_from_now(std::chrono::nanoseconds(WaitTimeNSec));
    PacketTimer.async_wait(std::bind(&TestUDPServer::handleNewPacket, this, std::placeholders::_1));
  }
}


