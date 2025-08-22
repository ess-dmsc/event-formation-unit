// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS Detector interface
///
//===----------------------------------------------------------------------===//

#pragma once

#include "common/StatCounterBase.h"
#include <CLI/CLI.hpp>
#include <common/Statistics.h>
#include <common/detector/BaseSettings.h>
#include <common/kafka/AR51Serializer.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/KafkaConfig.h>
#include <common/kafka/Producer.h>
#include <common/memory/RingBuffer.h>
#include <common/memory/SPSCFifo.h>
#include <common/readout/ess/Parser.h>
#include <cstdint>
#include <thread>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

struct ThreadInfo {
  std::function<void(void)> func;
  std::string name;
  std::thread thread;
};

class Detector {
protected:
  BaseSettings EFUSettings;
  Statistics Stats;

  struct ITCounters : public StatCounterBase {
    int64_t RxPackets{0};
    int64_t RxBytes{0};
    int64_t FifoPushErrors{0};
    int64_t RxIdle{0};
    int64_t TxRawReadoutPackets{0};
    int64_t FifoSeqErrors{0};

    ITCounters(Statistics &Stats)
        : StatCounterBase(Stats,
                          {{Detector::METRIC_RECEIVE_PACKETS, RxPackets},
                           {Detector::METRIC_RECEIVE_BYTES, RxBytes},
                           {Detector::METRIC_RECEIVE_DROPPED, FifoPushErrors},
                           {Detector::METRIC_FIFO_SEQ_ERRORS, FifoSeqErrors},
                           {Detector::METRIC_THREAD_INPUT_IDLE, RxIdle},
                           {Detector::METRIC_TRANSMIT_CALIBMODE_PACKETS,
                            TxRawReadoutPackets}}) {}
  } ITCounters;

public:
  // Static const strings for statistics names
  // Definition of static const strings for statistics names
  static const std::string METRIC_RECEIVE_PACKETS;
  static const std::string METRIC_RECEIVE_BYTES;
  static const std::string METRIC_RECEIVE_DROPPED;
  static const std::string METRIC_THREAD_INPUT_IDLE;
  static const std::string METRIC_TRANSMIT_CALIBMODE_PACKETS;
  static const std::string METRIC_FIFO_SEQ_ERRORS;

  using CommandFunction =
      std::function<int(std::vector<std::string>, char *, unsigned int *)>;
  using ThreadList = std::vector<ThreadInfo>;

  /// \brief Construct a Detector instance and initialize owned subsystems.
  ///
  /// This constructor stores the provided `BaseSettings` and constructs
  /// several helper objects that the detector owns and uses at runtime.
  ///
  /// Important owned objects created/configured by this constructor:
  ///  - `BaseSettings EFUSettings`
  ///  - `Statistics Stats`
  ///  - `ESSReadout::Parser ESSHeaderParser`
  ///  - `KafkaConfig KafkaCfg`
  ///  - `Producer MonitorProducer`
  ///  - `AR51Serializer MonitorSerializer`
  ///
  /// \note: take care with shutdown ordering — `MonitorSerializer` must not
  /// invoke its callback after `MonitorProducer` has been destroyed. Currently
  /// declaration order ensures, that MonitorSerializer is destroyed before
  /// MonitorProducer.
  /// \param settings BaseSettings object containing configuration parameters
  Detector(BaseSettings settings)
      : EFUSettings(settings),
        Stats(settings.GraphitePrefix, settings.GraphiteRegion),
        ITCounters(Stats), ESSHeaderParser(Stats),
        KafkaCfg(EFUSettings.KafkaConfigFile),
        MonitorProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaDebugTopic,
                        KafkaCfg.CfgParms, Stats, "monitor"),
        MonitorSerializer(
            EFUSettings.DetectorName,
            [this](const auto &DataBuffer, const auto &Timestamp) {
              MonitorProducer.produce(DataBuffer, Timestamp);
            }) {
    // ITCounters are now registered automatically via StatCounterBase
  }

  /// Receiving UDP data is now common across all detectors
  void inputThread();

  virtual ~Detector() = default;

  /// \brief returns the number of runtime counters (efustats)
  /// used by Parser.cpp for command query
  inline virtual int statsize() { return Stats.size(); }

  /// \brief returns a const reference to the name of the detector
  inline virtual const std::string &getDetectorName() const noexcept {
    return EFUSettings.DetectorName;
  }

  /// \brief returns the value of a runtime counter (efustat) based on its index
  /// used by Parser.cpp for command query
  inline virtual int64_t getStatValue(size_t index) const {
    return Stats.getValue(index);
  }

  /// \brief returns the value of a runtime counter (efustat) based on name
  /// used by Parser.cpp for command query
  inline virtual int64_t getStatValueByName(const std::string &name) const {
    return Stats.getValueByName(name);
  }

  /// \brief returns a constructed string object which holds the full name of a
  /// given stat together with its prefix
  inline virtual std::string getStatFullName(size_t index) const {
    return Stats.getFullName(index);
  }

  /// \brief returns a const reference to the prefix of a runtime counter
  /// (efustat) based on its index used by Parser.cpp for command query
  inline virtual const std::string &getStatPrefix(size_t index) const {
    return Stats.getStatPrefix(index);
  }

  /// \brief Getter for the input thread counters
  /// \return reference to the input thread counters structure
  inline const struct ITCounters &getInputCounters() const {
    return ITCounters;
  }

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

protected:
  ESSReadout::Parser ESSHeaderParser;
  KafkaConfig KafkaCfg;

private:
  Producer MonitorProducer;
  AR51Serializer MonitorSerializer;
};
