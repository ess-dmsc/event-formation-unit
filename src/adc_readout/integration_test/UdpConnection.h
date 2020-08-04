/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../AdcParse.h"
#include "DataPacket.h"
#include <common/Socket.h>

#include <atomic>
#include <deque>
#include <mutex>
#include <thread>

using TimePointNano = std::chrono::high_resolution_clock::time_point;
using TimeDurationNano = std::chrono::duration<size_t, std::nano>;

class UdpConnection {
public:
  UdpConnection(std::string DstAddress, std::uint16_t DstPort);
  ~UdpConnection();
  void waitTransmitDone();

  // used by the generators
  void addSamplingRun(void const *const DataPtr, size_t Bytes,
                      TimeStamp Timestamp);

  void transmitHeartbeat();

  bool shouldFlushIdleDataPacket(TimePointNano TimeNow);
  void flushIdleDataPacket(TimePointNano TimeNow);

  int getNrOfRuns() const { return SamplingRuns; };
  int getNrOfPackets() const { return PacketCount; };
  int getNrOfSentPackets() const { return SentPackets; };
  void incNumSentPackets() { SentPackets.store(SentPackets + 1); };

  const TimeDurationNano HeartbeatInterval{4'000'000'000ull};

  const TimeDurationNano DataIdleFlushInterval{500'000'000ull};

private:
#pragma pack(push, 2)
  struct IdlePacket_t {
    PacketHeader Head;
    IdleHeader Idle;
    std::uint16_t MagicSuffix1{0xEDFE};
    std::uint16_t MagicSuffix2{0x0DF0};
    void fixEndian() {
      Head.fixEndian();
      Idle.fixEndian();
    }
  };
#pragma pack(pop)

  struct TransmitRequest {
    bool IsData; /// \note if we need more types to be sent we could probably
                 /// put a "free request" callback in here.
    union {
      DataPacket *DataPtr;
      IdlePacket_t *IdlePtr;
    };

    TransmitRequest(DataPacket *DataPtr) : IsData(true), DataPtr(DataPtr) {}
    TransmitRequest(IdlePacket_t *IdlePtr) : IsData(false), IdlePtr(IdlePtr) {}

    void *getTxData();
    size_t getTxSize();
    void freePacket(UdpConnection *UdpCon);
  };

private:
  const std::uint64_t RefTimeDeltaNS{1'000'000'000ull / 14ull};
  std::uint64_t CurrentRefTimeNS{0};
  int SamplingRuns{0};
  const int MaxPacketSize{9000};
  std::string Address;
  std::uint16_t Port;

  const uint32_t KernelTxBufferSize{1'000'000};
  Socket::Endpoint RemoteEndpoint;
  UDPTransmitter DataSource;

  TimePointNano LastSampleDataAddTime;
  TimePointNano LastDataIdleFlushTime;

  void transmitThread();

  void queueTransmitRequest(TransmitRequest TR);
  void queueTransmitAndResetDataPacketBuilder();

  IdlePacket_t *allocIdlePacket();
  void freeIdlePacket(IdlePacket_t *idle);

  DataPacket *allocDataPacket();
  void freeDataPacket(DataPacket *data);

  std::uint16_t PacketCount{0};
  std::atomic_int SentPackets{0};

  std::thread TransmitThread;

  static const int TransmitQueueSize = 10;
  std::mutex TransmitRequestsAccess;
  std::deque<TransmitRequest> TransmitRequests;

  IdlePacket_t IdlePacketStore[3]; // there could potentially be false sharing
                                   // here by it would happen very rarely.
  std::mutex FreeIdlePacketsAccess;
  std::vector<IdlePacket_t *> FreeIdlePackets;

  DataPacket *DataPacketBuilder;

  std::mutex FreeDataPacketsAccess;
  std::vector<DataPacket *> FreeDataPackets;
};
