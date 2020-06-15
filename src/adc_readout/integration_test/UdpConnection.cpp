/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "UdpConnection.h"
#include <ciso646>
#include <functional>
#include <iostream>

using UdpEndpoint = asio::ip::udp::endpoint;
using namespace std::chrono_literals;

struct QueryResult {
  explicit QueryResult(asio::ip::udp::resolver::iterator &&Endpoints)
      : EndpointIterator(std::move(Endpoints)) {
    while (EndpointIterator != asio::ip::udp::resolver::iterator()) {
      auto CEndpoint = *EndpointIterator;
      EndpointList.push_back(CEndpoint);
      ++EndpointIterator;
    }
    std::sort(EndpointList.begin(), EndpointList.end(), [](auto &a, auto &b) {
      return a.address().is_v6() < b.address().is_v6();
    });
  }
  asio::ip::udp::endpoint getNextEndpoint() {
    if (NextEndpoint < EndpointList.size()) {
      return EndpointList[NextEndpoint++];
    }
    return {};
  }
  bool isDone() const { return NextEndpoint >= EndpointList.size(); }
  asio::ip::udp::resolver::iterator EndpointIterator;
  std::vector<UdpEndpoint> EndpointList;
  unsigned int NextEndpoint{0};
};

UdpConnection::UdpConnection(std::string DstAddress, std::uint16_t DstPort,
                             asio::io_service &Service)
    : Address(std::move(DstAddress)), Port(DstPort),
      Socket(Service, UdpEndpoint()), Resolver(Service),
      ReconnectTimeout(Service), HeartbeatTimeout(Service),
      DataPacketTimeout(Service),
      TransmitBuffer(std::make_unique<DataPacket>(MaxPacketSize)),
      SharedStandbyBuffer(std::make_unique<DataPacket>(MaxPacketSize)) {
  resolveDestination();

  IdlePacket.Head.Version = Protocol::VER_1;
  IdlePacket.Head.Type = PacketType::Idle;
  IdlePacket.Head.ReadoutLength = 20;
  IdlePacket.Head.ReadoutCount = 0;
  IdlePacket.Head.ClockMode = Clock::Clk_Ext;
  IdlePacket.Head.OversamplingFactor = 1;
  IdlePacket.Idle.TimeStamp = {0, 0};
  IdlePacket.fixEndian();
}

void UdpConnection::resolveDestination() {
  asio::ip::udp::resolver::query Query(asio::ip::udp::v4(), Address,
                                       std::to_string(Port));
  auto ResolveHandlerGlue = [this](auto &Error, auto EndpointIterator) {
    this->handleResolve(Error, EndpointIterator);
  };
  Resolver.async_resolve(Query, ResolveHandlerGlue);
}

void UdpConnection::reconnectWait() {
  auto HandlerGlue = [this](auto &) { this->resolveDestination(); };
  ReconnectTimeout.expires_after(1s);
  ReconnectTimeout.async_wait(HandlerGlue);
}

void UdpConnection::tryConnect(QueryResult AllEndpoints) {
  UdpEndpoint CurrentEndpoint = AllEndpoints.getNextEndpoint();
  auto HandlerGlue = [this, AllEndpoints](auto &Error) {
    this->handleConnect(Error, AllEndpoints);
  };
  Socket.async_connect(CurrentEndpoint, HandlerGlue);
}

void UdpConnection::handleResolve(
    const asio::error_code &Error,
    asio::ip::udp::resolver::iterator EndpointIter) {
  if (Error) {
    reconnectWait();
    return;
  }
  QueryResult AllEndpoints(std::move(EndpointIter));
  tryConnect(AllEndpoints);
}

void UdpConnection::handleConnect(const asio::error_code &Error,
                                  QueryResult const &AllEndpoints) {
  if (!Error) {
    startHeartbeatTimer();
    return;
  }
  Socket.close();
  if (AllEndpoints.isDone()) {
    reconnectWait();
    return;
  }
  tryConnect(AllEndpoints);
}

void UdpConnection::startHeartbeatTimer() {
  HeartbeatTimeout.expires_after(4s);
  HeartbeatTimeout.async_wait([this](auto &Error) {
    if (not Error) {
      transmitHeartbeat();
      startHeartbeatTimer();
    }
  });
}

void UdpConnection::startPacketTimer() {
  DataPacketTimeout.expires_after(500ms);
  DataPacketTimeout.async_wait([this](auto &Error) {
    if (not Error) {
      swapAndTransmitSharedStandbyBuffer();
      startHeartbeatTimer();
    }
  });
}

void UdpConnection::transmitHeartbeat() {
  TimeStamp IdleTS{CurrentRefTimeNS + RefTimeDeltaNS,
                   TimeStamp::ClockMode::External};
  auto IdleRawTS = RawTimeStamp{IdleTS.getSeconds(), IdleTS.getSecondsFrac()};
  IdleRawTS.fixEndian();
  IdlePacket.Head.ReferenceTimeStamp = IdleRawTS;
  IdlePacket.Idle.TimeStamp = IdleRawTS;

  transmitPacket(&IdlePacket, sizeof(IdlePacket));
}

void UdpConnection::transmitPacket(const void *DataPtr, const size_t Size) {
  Socket.async_send(asio::buffer(DataPtr, Size),
                    [this](auto &, auto) { this->incNumSentPackets(); });
}

// used by the generators
void UdpConnection::addSamplingRun(void const *const DataPtr, size_t Bytes,
                                   TimeStamp Timestamp) {
  if (CurrentRefTimeNS == 0) {
    CurrentRefTimeNS = Timestamp.getTimeStampNS();
  }

  // JonasNilsson:
  // I believe the code is trying to simulate the reference pulse timestamps of
  // the ESS accelerator. Those occur at fixed time intervals. Hence, when the
  // code determines that we are past the point in time at which we should have
  // a new reference pulse timestamp, it updates that timestamp.
  while (Timestamp.getTimeStampNS() > CurrentRefTimeNS + RefTimeDeltaNS) {
    CurrentRefTimeNS += RefTimeDeltaNS;
  }

  bool Success =
      SharedStandbyBuffer->addSamplingRun(DataPtr, Bytes, CurrentRefTimeNS);
  std::pair<size_t, size_t> BufferSizes = SharedStandbyBuffer->getBufferSizes();
  size_t Size = BufferSizes.first;
  size_t MaxSize = BufferSizes.second;

  if (not Success or MaxSize - Size < 20) {
    swapAndTransmitSharedStandbyBuffer();
    assert(
        SharedStandbyBuffer->addSamplingRun(DataPtr, Bytes, CurrentRefTimeNS));
  } else {
    startPacketTimer();
  }
  ++SamplingRuns;
  startHeartbeatTimer();
}

void UdpConnection::swapAndTransmitSharedStandbyBuffer() {
  std::swap(TransmitBuffer, SharedStandbyBuffer);
  SharedStandbyBuffer->resetPacket();
  auto DataInfo = TransmitBuffer->getBuffer(PacketCount);
  PacketCount++;
  transmitPacket(DataInfo.first, DataInfo.second);
}
