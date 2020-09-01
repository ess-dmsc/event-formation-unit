/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple UDP client implementation file.
 */

#include "UDPClient.h"
#include <common/Log.h>

using udp = asio::ip::udp;

UDPClient::UDPClient(std::shared_ptr<asio::io_service> const &IOService,
                     std::string const &Interface, std::uint16_t Port,
                     std::function<void(InData const &Packet)> Handler)
    : PacketHandler(std::move(Handler)), Socket(*IOService),
      Resolver(*IOService),
      EndPoint(asio::ip::address::from_string(Interface), Port) {
  try {
    Socket.open(asio::ip::udp::v4());
    Socket.bind(udp::endpoint(asio::ip::address::from_string(Interface), Port));
  } catch (std::runtime_error &Error) {
    LOG(INPUT, Sev::Error, "Unable to bind to local UDP port. Message is: {}",
        Error.what());
    Socket.close();
    return;
  }
  setupReceiver();
}

void UDPClient::setPacketHandler(
    std::function<void(InData const &Packet)> Handler) {
  PacketHandler = std::move(Handler);
}

void UDPClient::setupReceiver() {
  Socket.async_receive(asio::buffer(InputBuffer.Data, InData::MaxLength),
                       [this](auto &Error, auto BytesReceived) {
                         if (Error) {
                           LOG(INPUT, Sev::Warning,
                               "Error on receiving UDP packet. Message is: {}",
                               Error.message());
                           return;
                         }
                         InputBuffer.Length = BytesReceived;
                         PacketHandler(InputBuffer);
                         setupReceiver();
                       });
}
