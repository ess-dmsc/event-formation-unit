//
// Created by Jonas Nilsson on 2017-11-08.
//

#include <functional>
#include <iostream>
#include "UDPServer.h"
#include <ciso646>


UDPServer::UDPServer(std::uint16_t SrcPort, std::uint16_t DstPort)
    : Service(), Socket(Service, asio::ip::udp::endpoint(asio::ip::udp::v4(), SrcPort)), Resolver(Service), Work(new asio::io_service::work(Service)) {

      asio::ip::udp::resolver::query Query(asio::ip::udp::v4(), "localhost", std::to_string(DstPort));
  Resolver.async_resolve(Query, std::bind(&UDPServer::handleResolve, this, std::placeholders::_1, std::placeholders::_2));
  AsioThread = std::thread(&UDPServer::threadFunction, this);
}

UDPServer::~UDPServer() {
  Work.reset();
  AsioThread.join();
  Socket.close();
}

void UDPServer::handleWrite(const asio::error_code &Err, std::size_t __attribute__((unused)) BytesSent, BufferPtr Buffer) {
  if (Err) {
    ConnectionOk = false;
  }
  ++PacketsSent;
  Buffer.reset();
}

void UDPServer::threadFunction() {
  Service.run();
}

void UDPServer::handleResolve(const asio::error_code __attribute__((unused)) &Err, asio::ip::udp::resolver::iterator EndpointIter) {
  asio::ip::udp::endpoint Endpoint = *EndpointIter;
  Socket.async_connect(Endpoint, std::bind(&UDPServer::handleConnect, this, std::placeholders::_1, ++EndpointIter));
}

void UDPServer::handleConnect(const asio::error_code &Err,
                                  asio::ip::udp::resolver::iterator EndpointIter) {
  if (!Err) {
    ConnectionOk = true;
  }
  else if (EndpointIter != asio::ip::udp::resolver::iterator())
  {
    Socket.close();
    asio::ip::udp::endpoint Endpoint = *EndpointIter;
    Socket.async_connect(Endpoint,
                          std::bind(&UDPServer::handleConnect, this,
                                      std::placeholders::_1, ++EndpointIter));
  }
  
}

bool UDPServer::TransmitPacket(const std::uint8_t *DataPtr, const std::uint32_t Size) {
  if (not ConnectionOk) {
    return false;
  }
  BufferPtr Buffer(new std::uint8_t[Size], std::default_delete<std::uint8_t[]>());
  std::memcpy(Buffer.get(), DataPtr, Size);
  Service.post([=](){handlePacketTransmit(Buffer, Size);});
  return true;
}

void UDPServer::handlePacketTransmit(BufferPtr Buffer, const std::uint32_t Size) {
  Socket.async_send(asio::buffer(Buffer.get(), Size), std::bind(&UDPServer::handleWrite, this, std::placeholders::_1, std::placeholders::_2, Buffer));
}
