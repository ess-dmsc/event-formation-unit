// Copyright (C) 2016 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS Detector interface
///
//===----------------------------------------------------------------------===//

#pragma once

#include <CLI/CLI.hpp>
#include <atomic>
#include <common/Statistics.h>
#include <common/debug/Trace.h>
#include <common/detector/BaseSettings.h>
#include <common/memory/RingBuffer.h>
#include <common/memory/SPSCFifo.h>
#include <common/system/Socket.h>
#include <functional>
#include <common/debug/Log.h>
#include <map>
#include <memory>
#include <stdio.h>
#include <string>
#include <thread>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

struct ThreadInfo {
  std::function<void(void)> func;
  std::string name;
  std::thread thread;
};

class Detector {
public:
  struct {
    int64_t RxPackets{0};
    int64_t RxBytes{0};
    int64_t FifoPushErrors{0};
    int64_t RxIdle{0};
  } ITCounters; // Input Thread Counters

  using CommandFunction =
      std::function<int(std::vector<std::string>, char *, unsigned int *)>;
  using ThreadList = std::vector<ThreadInfo>;
  Detector(BaseSettings settings) : EFUSettings(settings), Stats(){};

  /// Receiving UDP data is now common across all detectors
  void inputThread() {
    XTRACE(INPUT, DEB, "Starting inputThread");
    Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                           EFUSettings.DetectorPort);

    UDPReceiver dataReceiver(local);
    dataReceiver.setBufferSizes(EFUSettings.TxSocketBufferSize,
                                EFUSettings.RxSocketBufferSize);
    dataReceiver.printBufferSizes();
    dataReceiver.setRecvTimeout(0, EFUSettings.SocketRxTimeoutUS);

    LOG(INIT, Sev::Info, "Detector input thread started on {}:{}", local.IpAddress,
        local.Port);

    while (runThreads) {
      int readSize;
      unsigned int rxBufferIndex = RxRingbuffer.getDataIndex();

      RxRingbuffer.setDataLength(rxBufferIndex, 0);
      if ((readSize =
               dataReceiver.receive(RxRingbuffer.getDataBuffer(rxBufferIndex),
                                    RxRingbuffer.getMaxBufSize())) > 0) {
        RxRingbuffer.setDataLength(rxBufferIndex, readSize);
        XTRACE(INPUT, DEB, "Received an udp packet of length %d bytes",
               readSize);
        ITCounters.RxPackets++;
        ITCounters.RxBytes += readSize;

        if (InputFifo.push(rxBufferIndex) == false) {
          ITCounters.FifoPushErrors++;
        } else {
          RxRingbuffer.getNextBuffer();
        }
      } else {
        ITCounters.RxIdle++;
      }
    }
    XTRACE(INPUT, ALW, "Stopping input thread.");
    return;
  }

  virtual ~Detector() = default;

  /// \brief returns the number of runtime counters (efustats)
  /// used by Parser.cpp for command query
  virtual int statsize() { return Stats.size(); }

  /// \brief returns the value of a runtime counter (efustat) based on its index
  /// used by Parser.cpp for command query
  virtual int64_t statvalue(size_t index) { return Stats.value(index); }

  /// \brief returns the value of a runtime counter (efustat) based on name
  /// used by Parser.cpp for command query
  virtual int64_t statvaluebyname(std::string name) {
    return Stats.valueByName(name);
  }

  /// \brief returns the name of a runtime counter (efustat) based on its index
  /// used by Parser.cpp for command query
  virtual std::string &statname(size_t index) { return Stats.name(index); }

  /// \brief return the current status mask (should be set in pipeline)
  virtual uint32_t runtimestat() { return RuntimeStatusMask; }

  virtual ThreadList &GetThreadInfo() { return Threads; };

  virtual void startThreads() {
    for (auto &tInfo : Threads) {
      tInfo.thread = std::thread(tInfo.func);
    }
  }

  virtual void stopThreads() {
    runThreads.store(false);
    for (auto &tInfo : Threads) {
      if (tInfo.thread.joinable()) {
        tInfo.thread.join();
      }
    }
  };

public:
  BaseSettings EFUSettings;
  Statistics Stats;
  
  /// \todo figure out the right size  of EthernetBufferMaxEntries
  static constexpr int EthernetBufferMaxEntries{2000};
  static constexpr int EthernetBufferSize{9000}; /// bytes
  static constexpr int KafkaBufferSize{12'400};  /// entries ~ 100kB

  /// Shared between input_thread and processing_thread
  memory_sequential_consistent::CircularFifo<unsigned int,
                                             EthernetBufferMaxEntries>
      InputFifo;
  /// \todo the number 11 is a workaround
  RingBuffer<EthernetBufferSize> RxRingbuffer{EthernetBufferMaxEntries + 11};

  // Ideally should match the CPU speed, but as this varies across
  // CPU versions we just select something in the 'middle'. This is
  // used to get an approximate time for periodic housekeeping so
  // it is not critical that this is precise.
  const int TSC_MHZ = 2900;

  void AddThreadFunction(std::function<void(void)> &func,
                         std::string funcName) {
    Threads.emplace_back(ThreadInfo{func, std::move(funcName), std::thread()});
  };

  void AddCommandFunction(std::string Name, CommandFunction FunctionObj) {
    DetectorCommands[Name] = FunctionObj;
  };

  ThreadList Threads;
  std::map<std::string, CommandFunction> DetectorCommands;
  std::atomic_bool runThreads{true};
  uint32_t RuntimeStatusMask{0};
};
