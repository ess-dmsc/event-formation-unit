/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "UdpConnection.h"

#include <common/Timer.h>
#include <unistd.h>
 
extern bool RunLoop;

UdpConnection::UdpConnection(std::string DstAddress, std::uint16_t DstPort)
    : Address(std::move(DstAddress)), Port(DstPort),
      RemoteEndpoint(Address, Port),
      DataSource(Socket::Endpoint("0.0.0.0", 0), RemoteEndpoint),
      DataPacketBuilder(nullptr) {

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

  for (int i = 0; i < TransmitQueueSize; i++) {
    FreeDataPackets.push_back(new DataPacket(MaxPacketSize));
    TransmitRequests.push_back(TransmitRequest((DataPacket *)nullptr));
  }
  DataPacketBuilder = FreeDataPackets.back();
  FreeDataPackets.pop_back();

  TransmitRequests.clear();

  TransmitThread = std::thread([this]() { this->transmitThread(); });
}

UdpConnection::~UdpConnection() { waitTransmitDone(); }

void UdpConnection::waitTransmitDone() {
  if (TransmitThread.joinable()) {
    TransmitThread.join();
  }
}

void UdpConnection::transmitHeartbeat() {
  IdlePacket_t *IdlePacket = allocIdlePacket();

  TimeStamp IdleTS{CurrentRefTimeNS + RefTimeDeltaNS,
                   TimeStamp::ClockMode::External};
  RawTimeStamp IdleRawTS{IdleTS.getSeconds(), IdleTS.getSecondsFrac()};
  IdleRawTS.fixEndian();
  IdlePacket->Head.ReferenceTimeStamp = IdleRawTS;
  IdlePacket->Idle.TimeStamp = IdleRawTS;

  queueTransmitRequest(TransmitRequest(IdlePacket));
}

bool UdpConnection::shouldFlushIdleDataPacket(TimePointNano TimeNow) {
  bool IdleFlushTimeoutExceeded =
      (TimeNow >= LastSampleDataAddTime + DataIdleFlushInterval);
  bool HasUnflushedData = (LastDataIdleFlushTime < LastSampleDataAddTime);
  if (0) {
    // clang-format off
    printf("IdleFlushTimeoutExceeded %i = (TimeNow %" PRIu64" >= LastSampleDataAddTime %" PRIu64" + DataIdleFlushInterval %zu)\n"
         "HasUnflushedData         %i = (LastDataIdleFlushTime %" PRIu64" <  LastSampleDataAddTime %" PRIu64")\n"
         , IdleFlushTimeoutExceeded ?1:0
         , TimeNow.time_since_epoch().count()
         , LastSampleDataAddTime.time_since_epoch().count()
         , DataIdleFlushInterval.count()
         , HasUnflushedData ?1:0
         , LastDataIdleFlushTime.time_since_epoch().count()
         , LastSampleDataAddTime.time_since_epoch().count());
    // clang-format on
  }
  return IdleFlushTimeoutExceeded && HasUnflushedData;
}

void UdpConnection::flushIdleDataPacket(TimePointNano TimeNow) {
  if (0) {
    printf("flushIdleDataPacket\n");
  }
  LastDataIdleFlushTime = TimeNow;
  queueTransmitAndResetDataPacketBuilder();
}

void UdpConnection::queueTransmitRequest(TransmitRequest TR) {
  TransmitRequestsAccess.lock();
  TransmitRequests.push_back(TR);
  TransmitRequestsAccess.unlock();
}

void *UdpConnection::TransmitRequest::getTxData() {
  if (IsData) {
    return DataPtr->Buffer.get();
  } else {
    return IdlePtr;
  }
}

size_t UdpConnection::TransmitRequest::getTxSize() {
  if (IsData) {
    return DataPtr->Size;
  } else {
    return sizeof(IdlePacket_t);
  }
}

void UdpConnection::TransmitRequest::freePacket(UdpConnection *UdpCon) {
  if (IsData) {
    UdpCon->freeDataPacket(DataPtr);
  } else {
    UdpCon->freeIdlePacket(IdlePtr);
  }
}

void UdpConnection::transmitThread() {
  static const bool ContinuousSpeedTest = false;
  static const bool RepeatPacketSpeedTest = false;
  bool FirstData = false;
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
      usleep(FirstData ? 1 : 10000);
      continue;
    } else {
      if (!FirstData) {
        FirstData = true;
        FirstDataTimer.now();
      }
    }

    void *DataBuf = TR.getTxData();
    std::size_t DataSize = TR.getTxSize();

    // ContinuousSpeedTest
    if (ContinuousSpeedTest) {
      for (;;) {
        DataSource.send(DataBuf, DataSize);

        if ((SendCount++ & ((1 << 18) - 1)) == 0) {
          uint64_t AccumSize = DataSize * SendCount;
          double Secs = FirstDataTimer.timeus() / 1e6;
          double GbPerSec = AccumSize / (1024 * 1024 * 1024 * Secs);
          double PkgPerSec = SendCount / Secs;
          const char *Name = "ContinuousSpeedTest";
          printf("Gb/sec %0.3f, pkg/sec %0.3f, pkg bytes %u [%s, %u]\n",
                 GbPerSec, PkgPerSec, (uint32_t)DataSize, Name, Port);
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
        const char *Name = "RepeatPacketSpeedTest";
        printf("Gb/sec %0.3f, pkg/sec %0.3f, pkg bytes %u [%s, %u]\n", GbPerSec,
               PkgPerSec, (uint32_t)DataSize, Name, Port);
      }
    } // normal send
    else {
      DataSource.send(DataBuf, DataSize);

      if ((SendCount++ & ((1 << 16) - 1)) == 0) {
        uint64_t AccumSize = DataSize * SendCount;
        double Secs = FirstDataTimer.timeus() / 1e6;
        double GbPerSec = AccumSize / (1024 * 1024 * 1024 * Secs);
        double PkgPerSec = SendCount / Secs;
        const char *Name = "normal send";
        printf("Gb/sec %0.3f, pkg/sec %0.3f, pkg bytes %u [%s, %u]\n", GbPerSec,
               PkgPerSec, (uint32_t)DataSize, Name, Port);
      }
    }

    this->incNumSentPackets();

    if (RepeatPacketSpeedTest) {
      queueTransmitRequest(TR);
    } else {
      TR.freePacket(this);
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
      DataPacketBuilder->addSamplingRun(DataPtr, Bytes, CurrentRefTimeNS);
  std::pair<size_t, size_t> BufferSizes = DataPacketBuilder->getBufferSizes();
  size_t Size = BufferSizes.first;
  size_t MaxSize = BufferSizes.second;

  if (not Success or MaxSize - Size < 20) {
    queueTransmitAndResetDataPacketBuilder();
    DataPacketBuilder->addSamplingRun(DataPtr, Bytes, CurrentRefTimeNS);
  }

  LastSampleDataAddTime = std::chrono::high_resolution_clock::now();
  ++SamplingRuns;
}

void UdpConnection::queueTransmitAndResetDataPacketBuilder() {
  DataPacket *SendPacket = DataPacketBuilder;
  DataPacketBuilder = nullptr;
  SendPacket->formatPacketForSend(PacketCount);
  queueTransmitRequest(TransmitRequest(SendPacket));
  PacketCount++;

  DataPacketBuilder = allocDataPacket();
  DataPacketBuilder->resetPacket();
}

UdpConnection::IdlePacket_t *UdpConnection::allocIdlePacket() {
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

void UdpConnection::freeIdlePacket(IdlePacket_t *idle) {
  FreeIdlePacketsAccess.lock();
  FreeIdlePackets.push_back(idle);
  FreeIdlePacketsAccess.unlock();
}

DataPacket *UdpConnection::allocDataPacket() {
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

void UdpConnection::freeDataPacket(DataPacket *data) {
  FreeDataPacketsAccess.lock();
  FreeDataPackets.push_back(data);
  FreeDataPacketsAccess.unlock();
}