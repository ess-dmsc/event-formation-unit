/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "UdpConnection.h"

#include <common/Timer.h>
#include <unistd.h>

static const int kTransmitQueueSize = 10;

extern bool RunLoop;

UdpConnection::UdpConnection(std::string DstAddress, std::uint16_t DstPort)
    : Address(std::move(DstAddress)), Port(DstPort),
      RemoteEndpoint(Address, Port),
      DataSource(Socket::Endpoint("0.0.0.0", 0), RemoteEndpoint),
      SharedStandbyBuffer(nullptr) {

  DataSource.setBufferSizes(KernelTxBufferSize, 0);
  DataSource.printBufferSizes();

  for (auto &IdlePacket : IdlePacketStore) {
    IdlePacket.Head.Version = Protocol::VER_1;
    IdlePacket.Head.Type = PacketType::Idle;
    IdlePacket.Head.ReadoutLength = 20;
    IdlePacket.Head.ReadoutCount = 0;
    IdlePacket.Head.ClockMode = Clock::Clk_Ext;
    IdlePacket.Head.OversamplingFactor = 1;
    IdlePacket.Idle.TimeStamp = {0, 0};
    IdlePacket.fixEndian();
    FreeIdlePackets.push_back(&IdlePacket);
    TransmitRequests.push_back(TransmitRequest((IdlePacket_t *)nullptr));
  }

  for (int i = 0; i < kTransmitQueueSize; i++) {
    FreeDataPackets.push_back(new DataPacket(MaxPacketSize));
    TransmitRequests.push_back(TransmitRequest((DataPacket *)nullptr));
  }
  SharedStandbyBuffer = FreeDataPackets.back();
  FreeDataPackets.pop_back();

  TransmitRequests.clear();

  TransmitThread = std::thread([this]() { this->transmitThread(); });
}

UdpConnection::~UdpConnection() {
  if (TransmitThread.joinable()) {
    TransmitThread.join();
  }
}

void UdpConnection::startHeartbeatTimer() {
  // HeartbeatTimeout.expires_after(4s);
  // HeartbeatTimeout.async_wait([this](auto &Error) {
  //  if (not Error) {
  //    transmitHeartbeat();
  //    startHeartbeatTimer();
  //  }
  //});
}

void UdpConnection::startPacketTimer() {
  // DataPacketTimeout.expires_after(500ms);
  // DataPacketTimeout.async_wait([this](auto &Error) {
  //  if (not Error) {
  //    swapAndTransmitSharedStandbyBuffer();
  //    startHeartbeatTimer();
  //  }
  //});
}

void UdpConnection::transmitHeartbeat() {
  IdlePacket_t *IdlePacket = AllocIdlePacket();

  TimeStamp IdleTS{CurrentRefTimeNS + RefTimeDeltaNS,
                   TimeStamp::ClockMode::External};

  RawTimeStamp IdleRawTS{IdleTS.getSeconds(), IdleTS.getSecondsFrac()};
  IdleRawTS.fixEndian();

  IdlePacket->Head.ReferenceTimeStamp = IdleRawTS;
  IdlePacket->Idle.TimeStamp = IdleRawTS;
  queueTransmitRequest(TransmitRequest(IdlePacket));
}

void UdpConnection::queueTransmitRequest(TransmitRequest TR) {
  TransmitRequestsAccess.lock();
  TransmitRequests.push_back(TR);
  TransmitRequestsAccess.unlock();
}

void *UdpConnection::TransmitRequest::GetTxData() {
  if (IsData) {
    return DataPtr->Buffer.get();
  } else {
    return IdlePtr;
  }
}

size_t UdpConnection::TransmitRequest::GetTxSize() {
  if (IsData) {
    return DataPtr->Size;
  } else {
    return sizeof(IdlePacket_t);
  }
}

void UdpConnection::TransmitRequest::FreePacket(UdpConnection *UdpCon) {
  if (IsData) {
    UdpCon->FreeDataPacket(DataPtr);
  } else {
    UdpCon->FreeIdlePacket(IdlePtr);
  }
}

void UdpConnection::transmitThread() {

  static const bool ContinuousSpeedTest = false;
  if (ContinuousSpeedTest) {
    fprintf(stdout, "ContinuousSpeedTest\n");
  }
  static const bool RepeatPacketSpeedTest = false;
  if (RepeatPacketSpeedTest) {
    fprintf(stdout, "RepeatPacketSpeedTest\n");
  }
  bool FirstData = true;
  Timer FirstDataTimer;
  uint64_t SendCount = 0;

  while (RunLoop) {
    bool empty = false;
    TransmitRequest TR = (IdlePacket_t *)nullptr; // default

    TransmitRequestsAccess.lock();
    empty = (TransmitRequests.size() == 0);
    if (!empty) {
      TR = TransmitRequests.front();
      TransmitRequests.pop_front();
    }
    TransmitRequestsAccess.unlock();

    if (empty) {
      usleep(FirstData ? 10000 : 1);
      continue;
    } else {
      if (FirstData) {
        FirstData = false;
        FirstDataTimer.now();
      }
    }

    void *DataBuf = TR.GetTxData();
    std::size_t DataSize = TR.GetTxSize();

    // ContinuousSpeedTest
    if (ContinuousSpeedTest) {
      for (;;) {
        DataSource.send(DataBuf, DataSize);

        if ((SendCount++ & ((1 << 18) - 1)) == 0) {
          uint64_t AccumSize = DataSize * SendCount;
          double Secs = FirstDataTimer.timeus() / 1e6;
          double GbPerSec = AccumSize / (1024 * 1024 * 1024 * Secs);
          double PkgPerSec = SendCount / Secs;
          fprintf(stderr, "GbPerSec %0.3f, pkg/sec %0.3f, pkg bytes %u\n",
                  GbPerSec, PkgPerSec, (uint32_t)DataSize);
        }
      }
    } // RepeatPacketSpeedTest
    else if (RepeatPacketSpeedTest) {
      DataSource.send(DataBuf, DataSize);

      if ((SendCount++ & ((1 << 17) - 1)) == 0) {
        uint64_t AccumSize = DataSize * SendCount;
        double Secs = FirstDataTimer.timeus() / 1e6;
        double GbPerSec = AccumSize / (1024 * 1024 * 1024 * Secs);
        double PkgPerSec = SendCount / Secs;
        fprintf(stderr, "GbPerSec %0.3f, pkg/sec %0.3f, pkg bytes %u\n",
                GbPerSec, PkgPerSec, (uint32_t)DataSize);
      }
    } // normal send
    else {
      DataSource.send(DataBuf, DataSize);

      if ((SendCount++ & ((1 << 16) - 1)) == 0) {
        uint64_t AccumSize = DataSize * SendCount;
        double Secs = FirstDataTimer.timeus() / 1e6;
        double GbPerSec = AccumSize / (1024 * 1024 * 1024 * Secs);
        double PkgPerSec = SendCount / Secs;
        fprintf(stderr, "GbPerSec %0.3f, pkg/sec %0.3f, pkg bytes %u\n",
                GbPerSec, PkgPerSec, (uint32_t)DataSize);
      }
    }

    this->incNumSentPackets();

    if (RepeatPacketSpeedTest) {
      queueTransmitRequest(TR);
    } else {
      TR.FreePacket(this);
    }
  }
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
    SharedStandbyBuffer->addSamplingRun(DataPtr, Bytes, CurrentRefTimeNS);
  } else {
    // startPacketTimer();
  }
  ++SamplingRuns;
  // startHeartbeatTimer();
}

void UdpConnection::swapAndTransmitSharedStandbyBuffer() {
  DataPacket *TransmitBuffer = SharedStandbyBuffer;
  SharedStandbyBuffer = nullptr;
  TransmitBuffer->formatPacketForSend(PacketCount);
  PacketCount++;

  queueTransmitRequest(TransmitRequest(TransmitBuffer));

  SharedStandbyBuffer = AllocDataPacket();
  SharedStandbyBuffer->resetPacket();
}

UdpConnection::IdlePacket_t *UdpConnection::AllocIdlePacket() {
  IdlePacket_t *IdlePacket = nullptr;
  while (IdlePacket == nullptr) {
    FreeIdlePacketsAccess.lock();
    if (FreeIdlePackets.size()) {
      IdlePacket = FreeIdlePackets.back();
      FreeIdlePackets.pop_back();
    }
    FreeIdlePacketsAccess.unlock();
    if (IdlePacket == nullptr) {
      usleep(10); // this is very unlikely
    }
  }
  return IdlePacket;
}

void UdpConnection::FreeIdlePacket(IdlePacket_t *idle) {
  FreeIdlePacketsAccess.lock();
  FreeIdlePackets.push_back(idle);
  FreeIdlePacketsAccess.unlock();
}

DataPacket *UdpConnection::AllocDataPacket() {
  DataPacket *DataPacket = nullptr;
  uint32_t EmptyStreakCount = 0;
  while (DataPacket == nullptr) {
    FreeDataPacketsAccess.lock();
    if (FreeDataPackets.size()) {
      DataPacket = FreeDataPackets.back();
      FreeDataPackets.pop_back();
      EmptyStreakCount = 0;
    } else {
      EmptyStreakCount++;
    }
    FreeDataPacketsAccess.unlock();

    if (DataPacket == nullptr) {
      uint32_t waitUSec = 1;
      if (EmptyStreakCount > 10)
        waitUSec = 10;
      if (EmptyStreakCount > 100)
        waitUSec = 100;
      usleep(waitUSec);
    }
  }
  return DataPacket;
}

void UdpConnection::FreeDataPacket(DataPacket *data) {
  FreeDataPacketsAccess.lock();
  FreeDataPackets.push_back(data);
  FreeDataPacketsAccess.unlock();
}