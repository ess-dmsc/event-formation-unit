/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wunused-variable"
#ifndef assert
#define assert(...) /**/
#endif
#include <asio.hpp>
#pragma GCC diagnostic pop

#include "../AdcParse.h"
#include "DataPacket.h"
#include <map>

using SocketPtr = std::shared_ptr<asio::ip::udp::socket>;
using WorkPtr = std::unique_ptr<asio::io_service::work>;
using BufferPtr = std::shared_ptr<std::uint8_t>;

struct QueryResult;

class FPGASim {
public:
  FPGASim(std::string DstAddress, std::uint16_t DstPort,
          asio::io_service &Service);
  ~FPGASim() = default;

  void addSamplingRun(void const *const DataPtr, size_t Bytes,
                      TimeStamp Timestamp);
  int getNrOfRuns() const { return SamplingRuns; };
  int getNrOfPackets() const { return PacketCount; };
  int getNrOfSentPackets() const { return SentPackets; };
  void incNumSentPackets() { SentPackets.store(SentPackets + 1); };

private:
  const std::uint64_t RefTimeDeltaNS{1000000000ull / 14ull};
  std::uint64_t CurrentRefTimeNS{0};
  int SamplingRuns{0};
  const int MaxPacketSize{9000};
  std::string Address;
  std::uint16_t Port;
  asio::ip::udp::socket Socket;
  asio::ip::udp::resolver Resolver;

  void resolveDestination();
  void reconnectWait();
  void tryConnect(QueryResult AllEndpoints);
  void startHeartbeatTimer();
  void startPacketTimer();
  void transmitStandbyBuffer();
  void transmitPacket(const void *DataPtr, const size_t Size);

  void transmitHeartbeat();

  void handleResolve(const asio::error_code &Error,
                     asio::ip::udp::resolver::iterator EndpointIter);
  void handleHeartbeat(const asio::error_code &Error);
  void handleConnect(const asio::error_code &Error,
                     QueryResult const &AllEndpoints);

  void handlePacketTimeout(const asio::error_code &Error);

  asio::system_timer ReconnectTimeout;
  asio::system_timer HeartbeatTimeout;
  asio::system_timer DataPacketTimeout;
  std::uint16_t PacketCount{0};
  std::atomic_int SentPackets{0};
  std::unique_ptr<DataPacket> TransmitBuffer;
  std::unique_ptr<DataPacket> StandbyBuffer;

#pragma pack(push, 2)
  struct {
    PacketHeader Head;
    IdleHeader Idle;
    std::uint16_t MagicSuffix1{0xEDFE};
    std::uint16_t MagicSuffix2{0x0DF0};
    void fixEndian() {
      Head.fixEndian();
      Idle.fixEndian();
    }
  } IdlePacket;
#pragma pack(pop)
};
