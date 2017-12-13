//
// Created by Jonas Nilsson on 2017-11-08.
//

#include <boost/bind.hpp>
#include <iostream>
#include "TestUDPServer.h"
#include <ciso646>

TestUDPServer::TestUDPServer(std::uint16_t SrcPort, std::uint16_t DstPort, std::uint32_t Packets, std::uint32_t WaitTimeNS)
    : Service(), Socket(Service, asio::ip::udp::endpoint(asio::ip::udp::v4(), SrcPort)), Resolver(Service), NrOfPackets(Packets), WaitTimeNSec(WaitTimeNS), PacketTimer(Service) {

      asio::ip::udp::resolver::query Query(asio::ip::udp::v4(), "localhost", std::to_string(DstPort));
  Resolver.async_resolve(Query, boost::bind(&TestUDPServer::handleResolve, this, asio::placeholders::error, asio::placeholders::iterator));
  AsioThread = std::thread(&TestUDPServer::threadFunction, this);
}

TestUDPServer::~TestUDPServer() {
  Service.stop();
  AsioThread.join();
  Socket.close();
}

void TestUDPServer::handleWrite(const boost::system::error_code &Err, std::size_t __attribute__((unused)) BytesSent) {
  if (Err) {
    GotError = true;
  }
}

void TestUDPServer::threadFunction() {
  Service.run();
}

void TestUDPServer::handleResolve(const boost::system::error_code __attribute__((unused)) &Err, asio::ip::udp::resolver::iterator EndpointIter) {
  asio::ip::udp::endpoint Endpoint = *EndpointIter;
  Socket.async_connect(Endpoint, boost::bind(&TestUDPServer::handleConnect, this, asio::placeholders::error, ++EndpointIter));
}

void TestUDPServer::handleConnect(const boost::system::error_code &Err,
                                  asio::ip::udp::resolver::iterator EndpointIter) {
  if (!Err) {
    Socket.async_send(asio::buffer(SendBuffer, BufferSize), boost::bind(&TestUDPServer::handleWrite, this, asio::placeholders::error,
                                                                        asio::placeholders::bytes_transferred));
    --NrOfPackets;
    PacketTimer.expires_from_now(std::chrono::nanoseconds(WaitTimeNSec));
    PacketTimer.async_wait(boost::bind(&TestUDPServer::handleNewPacket, this, asio::placeholders::error));
  }
  else if (EndpointIter != asio::ip::udp::resolver::iterator())
  {
    Socket.close();
    asio::ip::udp::endpoint Endpoint = *EndpointIter;
    Socket.async_connect(Endpoint,
                          boost::bind(&TestUDPServer::handleConnect, this,
                                      boost::asio::placeholders::error, ++EndpointIter));
  }
}

void TestUDPServer::handleNewPacket(const boost::system::error_code &Err) {
  if (!Err and NrOfPackets > 0 and not GotError) {
    --NrOfPackets;
    Socket.async_send(asio::buffer(SendBuffer, BufferSize), boost::bind(&TestUDPServer::handleWrite, this, asio::placeholders::error,
                                                                        asio::placeholders::bytes_transferred));
    PacketTimer.expires_from_now(std::chrono::nanoseconds(WaitTimeNSec));
    PacketTimer.async_wait(boost::bind(&TestUDPServer::handleNewPacket, this, asio::placeholders::error));
  }
}


