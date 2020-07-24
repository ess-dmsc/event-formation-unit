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

#include <deque>
#include <mutex>
#include <thread>

class UdpConnection {
public:
  UdpConnection(std::string DstAddress, std::uint16_t DstPort);
  ~UdpConnection();

  // used by the generators
  void addSamplingRun(void const *const DataPtr, size_t Bytes,
                      TimeStamp Timestamp);

  int getNrOfRuns() const { return SamplingRuns; };
  int getNrOfPackets() const { return PacketCount; };
  int getNrOfSentPackets() const { return SentPackets; };
  void incNumSentPackets() { SentPackets.store(SentPackets + 1); };

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

  friend struct TransmitRequest;
  struct TransmitRequest {
    bool IsData; /// \note if we need more types to be sent we could probably
                 /// put a "free request" callback in here.
    union {
      DataPacket *DataPtr;
      IdlePacket_t *IdlePtr;
    };

    TransmitRequest(DataPacket *DataPtr) : IsData(true), DataPtr(DataPtr) {}
    TransmitRequest(IdlePacket_t *IdlePtr) : IsData(false), IdlePtr(IdlePtr) {}

    void *GetTxData();
    size_t GetTxSize();
    void FreePacket(UdpConnection *UdpCon);
  };

private:
  const std::uint64_t RefTimeDeltaNS{1000000000ull / 14ull};
  std::uint64_t CurrentRefTimeNS{0};
  int SamplingRuns{0};
  const int MaxPacketSize{9000};
  std::string Address;
  std::uint16_t Port;

  const uint32_t KernelTxBufferSize{1000000};
  Socket::Endpoint RemoteEndpoint;
  UDPTransmitter DataSource;

  void transmitThread();

  void startHeartbeatTimer();
  void startPacketTimer();
  void swapAndTransmitSharedStandbyBuffer();
  void queueTransmitRequest(TransmitRequest TR);

  IdlePacket_t *AllocIdlePacket();
  void FreeIdlePacket(IdlePacket_t *idle);

  DataPacket *AllocDataPacket();
  void FreeDataPacket(DataPacket *data);

  void transmitHeartbeat();

  std::uint16_t PacketCount{0};
  std::atomic_int SentPackets{0};

  std::thread TransmitThread;

  std::mutex TransmitRequestsAccess;
  std::deque<TransmitRequest> TransmitRequests;

  IdlePacket_t IdlePacketStore[3]; // there could potentially be false sharing
                                   // here by it would happen very rarely.
  std::mutex FreeIdlePacketsAccess;
  std::vector<IdlePacket_t *> FreeIdlePackets;

  DataPacket *SharedStandbyBuffer;

  std::mutex FreeDataPacketsAccess;
  std::vector<DataPacket *> FreeDataPackets;
};
