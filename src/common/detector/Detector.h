// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS Detector interface
///
//===----------------------------------------------------------------------===//

#pragma once

#include <CLI/CLI.hpp>
#include <common/Statistics.h>
#include <common/detector/BaseSettings.h>
#include <common/kafka/AR51Serializer.h>
#include <common/kafka/EV44Serializer.h>
#include <common/memory/RingBuffer.h>
#include <common/memory/SPSCFifo.h>
#include <common/readout/ess/Parser.h>
#include <thread>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

struct ThreadInfo {
  std::function<void(void)> func;
  std::string name;
  std::thread thread;
};

class Detector {
private:
  struct {
    int64_t RxPackets{0};
    int64_t RxBytes{0};
    int64_t FifoPushErrors{0};
    int64_t RxIdle{0};
    int64_t CalibModePackets{0};
  } ITCounters; // Input Thread Counters

public:
  using CommandFunction =
      std::function<int(std::vector<std::string>, char *, unsigned int *)>;
  using ThreadList = std::vector<ThreadInfo>;

  /// \brief Constructor for the Detector class
  /// \param settings BaseSettings object containing configuration parameters
  Detector(BaseSettings settings)
      : EFUSettings(settings), Stats(), ESSHeaderParser(Stats) {
    Stats.create("receive.packets", ITCounters.RxPackets);
    Stats.create("receive.bytes", ITCounters.RxBytes);
    Stats.create("receive.dropped", ITCounters.FifoPushErrors);
  }

  /// Receiving UDP data is now common across all detectors
  void inputThread();

  virtual ~Detector() = default;

  /// \brief returns the number of runtime counters (efustats)
  /// used by Parser.cpp for command query
  inline virtual int statsize() { return Stats.size(); }

  /// \brief returns the value of a runtime counter (efustat) based on its index
  /// used by Parser.cpp for command query
  inline virtual int64_t statvalue(size_t index) { return Stats.value(index); }

  /// \brief returns the value of a runtime counter (efustat) based on name
  /// used by Parser.cpp for command query
  inline virtual int64_t statvaluebyname(const std::string &name) {
    return Stats.valueByName(name);
  }

  /// \brief returns the name of a runtime counter (efustat) based on its index
  /// used by Parser.cpp for command query
  inline virtual std::string &statname(size_t index) {
    return Stats.name(index);
  }

  /// \brief Getter for the input thread counters
  /// \return reference to the input thread counters structure
  inline auto &getInputCounters() { return ITCounters; }

  /// \brief return the current status mask (should be set in pipeline)
  inline virtual uint32_t runtimestat() { return RuntimeStatusMask; }

  inline virtual ThreadList &GetThreadInfo() { return Threads; }

  virtual void startThreads();

  virtual void stopThreads();

  inline void AddThreadFunction(std::function<void(void)> &func,
                                std::string funcName) {
    Threads.emplace_back(ThreadInfo{func, std::move(funcName), std::thread()});
  }

  inline void AddCommandFunction(const std::string &Name,
                                 CommandFunction FunctionObj) {
    DetectorCommands[Name] = FunctionObj;
  }

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

  ThreadList Threads;
  std::map<std::string, CommandFunction> DetectorCommands;
  std::atomic_bool runThreads{true};
  uint32_t RuntimeStatusMask{0};
  bool CalibrationMode{false};

  // Common to all EFUs
  std::unique_ptr<EV44Serializer> Serializer;
  std::unique_ptr<AR51Serializer> MonitorSerializer;

protected:
  ESSReadout::Parser ESSHeaderParser;
};
