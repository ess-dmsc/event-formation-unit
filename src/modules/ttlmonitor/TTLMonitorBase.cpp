// Copyright (C) 2022 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TTLMonitor instrument base plugin
///
//===----------------------------------------------------------------------===//

#include "common/detector/Detector.h"
#include <cinttypes>
#include <common/debug/Trace.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/KafkaConfig.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/time/TimeString.h>

#include <memory>
#include <unistd.h>

#include <common/RuntimeStat.h>
#include <common/memory/SPSCFifo.h>
#include <common/system/Socket.h>
#include <common/time/TimeString.h>
#include <common/time/Timer.h>
#include <stdio.h>
#include <ttlmonitor/TTLMonitorBase.h>
#include <ttlmonitor/TTLMonitorInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace TTLMonitor {

const char *classname = "TTLMonitor detector with ESS readout";

TTLMonitorBase::TTLMonitorBase(BaseSettings const &settings)
    : Detector(settings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off

  // Rx and Tx stats
  Stats.create("receive.packets", ITCounters.RxPackets);
  Stats.create("receive.bytes", ITCounters.RxBytes);
  Stats.create("receive.dropped", ITCounters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);
  Stats.create("transmit.bytes", Counters.TxBytes);

  // ESS Readout header stats
  Stats.create("essheader.error_header", Counters.ErrorESSHeaders);
  Stats.create("essheader.error_buffer", Counters.ReadoutStats.ErrorBuffer);
  Stats.create("essheader.error_cookie", Counters.ReadoutStats.ErrorCookie);
  Stats.create("essheader.error_pad", Counters.ReadoutStats.ErrorPad);
  Stats.create("essheader.error_size", Counters.ReadoutStats.ErrorSize);
  Stats.create("essheader.error_version", Counters.ReadoutStats.ErrorVersion);
  Stats.create("essheader.error_output_queue", Counters.ReadoutStats.ErrorOutputQueue);
  Stats.create("essheader.error_type", Counters.ReadoutStats.ErrorTypeSubType);
  Stats.create("essheader.error_seqno", Counters.ReadoutStats.ErrorSeqNum);
  Stats.create("essheader.error_timehigh", Counters.ReadoutStats.ErrorTimeHigh);
  Stats.create("essheader.error_timefrac", Counters.ReadoutStats.ErrorTimeFrac);
  Stats.create("essheader.heartbeats", Counters.ReadoutStats.HeartBeats);

  //
  Stats.create("monitors.count", Counters.MonitorCounts);
  Stats.create("monitors.reduced", Counters.MonitorIgnored);
  Stats.create("readouts.adc_max", Counters.MaxADC);
  Stats.create("readouts.tof_toolarge", Counters.TOFErrors);
  Stats.create("readouts.ring_mismatch", Counters.RingCfgErrors);
  Stats.create("readouts.fen_mismatch", Counters.FENCfgErrors);
  Stats.create("readouts.channel_errors", Counters.ChannelCfgErrors);

  Stats.create("readouts.error_size", Counters.TTLMonStats.ErrorSize);
  Stats.create("readouts.error_fiber", Counters.TTLMonStats.ErrorFiber);
  Stats.create("readouts.error_fen", Counters.TTLMonStats.ErrorFEN);
  Stats.create("readouts.error_adc", Counters.TTLMonStats.ErrorADC);
  Stats.create("readouts.error_datalen", Counters.TTLMonStats.ErrorDataLength);
  Stats.create("readouts.error_timefrac", Counters.TTLMonStats.ErrorTimeFrac);
  Stats.create("readouts.count", Counters.TTLMonStats.Readouts);
  Stats.create("readouts.empty", Counters.TTLMonStats.NoData);


  // Time stats
  Stats.create("readouts.tof_count", Counters.TimeStats.TofCount);
  Stats.create("readouts.tof_neg", Counters.TimeStats.TofNegative);
  Stats.create("readouts.prevtof_count", Counters.TimeStats.PrevTofCount);
  Stats.create("readouts.prevtof_neg", Counters.TimeStats.PrevTofNegative);

  //
  Stats.create("thread.receive_idle", ITCounters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  /// \todo below stats are common to all detectors
  Stats.create("kafka.produce_fails", Counters.KafkaStats.produce_fails);
  Stats.create("kafka.ev_errors", Counters.KafkaStats.ev_errors);
  Stats.create("kafka.ev_others", Counters.KafkaStats.ev_others);
  Stats.create("kafka.dr_errors", Counters.KafkaStats.dr_errors);
  Stats.create("kafka.dr_others", Counters.KafkaStats.dr_noerrors);
  // clang-format on
  std::function<void()> inputFunc = [this]() { inputThread(); };
  AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    TTLMonitorBase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d TTLMonitor Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void TTLMonitorBase::processing_thread() {

  if (EFUSettings.KafkaTopic == "") {
    XTRACE(INPUT, ALW, "Missing topic - mandatory for ttl monitor");
    stopThreads();
  }
  XTRACE(INPUT, ALW, "Kafka topic %s", EFUSettings.KafkaTopic.c_str());

  // Event producer
  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer eventprod(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                     KafkaCfg.CfgParms);
  auto Produce = [&eventprod](auto DataBuffer, auto Timestamp) {
    eventprod.produce(DataBuffer, Timestamp);
  };

  TTLMonitorInstrument TTLMonitor(Counters, EFUSettings);

  for (int i = 0; i < TTLMonitor.Conf.Parms.NumberOfMonitors; ++i) {
    // Create a serializer for each monitor

    // KafkaBufferSize is not yet initialized before the unique_ptr, so I
    // enforce to have it locally.
    /// \todo This is a bit of a hack, we should find a nice way to enforce the
    /// initialization of KafkaBufferSize before the unique_ptr
    int kafkaBufferSize = KafkaBufferSize;
    SerializersPtr.push_back(std::make_unique<EV44Serializer>(
        kafkaBufferSize, "ttlmon" + std::to_string(i), Produce));
  }

  for (auto &serializerPtr : SerializersPtr) {
    TTLMonitor.SerializersPtr.push_back(serializerPtr.get());
  }

  unsigned int DataIndex;
  // Monitor these counters
  TSCTimer ProduceTimer(EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ);
  RuntimeStat RtStat(
      {ITCounters.RxPackets, Counters.MonitorCounts, Counters.TxBytes});

  while (runThreads) {
    if (InputFifo.pop(DataIndex)) { // There is data in the FIFO - do processing
      auto DataLen = RxRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto DataPtr = RxRingbuffer.getDataBuffer(DataIndex);

      int64_t SeqErrOld = Counters.ReadoutStats.ErrorSeqNum;
      auto Res = TTLMonitor.ESSReadoutParser.validate(
          DataPtr, DataLen, TTLMonitor.Conf.Parms.TypeSubType);
      Counters.ReadoutStats = TTLMonitor.ESSReadoutParser.Stats;

      if (SeqErrOld != Counters.ReadoutStats.ErrorSeqNum) {
        XTRACE(DATA, WAR, "SeqNum error at RxPackets %" PRIu64,
               ITCounters.RxPackets);
      }

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, WAR,
               "Error parsing ESS readout header (RxPackets %" PRIu64 ")",
               ITCounters.RxPackets);
        // hexDump(DataPtr, std::min(64, DataLen));
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      TTLMonitor.TTLMonParser.parse(TTLMonitor.ESSReadoutParser.Packet);
      Counters.TTLMonStats = TTLMonitor.TTLMonParser.Stats;
      Counters.TimeStats = TTLMonitor.ESSReadoutParser.Packet.Time.Stats;

      TTLMonitor.processMonitorReadouts();

    } else {
      // There is NO data in the FIFO - increment idle counter and sleep a
      // little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    // Not only flush serializer data but also update runtime stats
    if (ProduceTimer.timeout()) {
      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {ITCounters.RxPackets, Counters.MonitorCounts, Counters.TxBytes});

      for (auto &serializer : SerializersPtr) {
        XTRACE(DATA, DEB, "Serializer timed out, producing message now");
        Counters.TxBytes += serializer->produce();
      }
      Counters.KafkaStats = eventprod.stats;
    } // ProduceTimer
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

} // namespace TTLMonitor
