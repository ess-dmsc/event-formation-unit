/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple UDP client header file.
///
//===----------------------------------------------------------------------===//

#pragma once
#include "AdcBufferElements.h"
#include <asio.hpp>
#include <cstdint>
#include <memory>
#include <string>

class UDPClient {
public:
  UDPClient(std::shared_ptr<asio::io_service> const &IOService,
            std::string const &Interface, std::uint16_t Port,
            std::function<void(InData const &Packet)> Handler);
  virtual ~UDPClient() = default;

protected:
  InData InputBuffer;
  void receiveHandler(const asio::error_code &Error, std::size_t BytesReceived);

  std::function<void(InData const &Packet)> PacketHandler;
  asio::ip::udp::socket Socket;
  asio::ip::udp::resolver Resolver;
  asio::ip::udp::endpoint EndPoint;
  void setupReceiver();
};
