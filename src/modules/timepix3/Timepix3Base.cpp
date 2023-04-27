// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of the detector pipeline plugin for Timepix3
/// detectors
//===----------------------------------------------------------------------===//

#include "timepix3/Timepix3Base.h"

#include <cinttypes>
#include <common/RuntimeStat.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/KafkaConfig.h>
#include <common/system/Socket.h>
#include <common/time/TSCTimer.h>
#include <common/time/TimeString.h>
#include <common/time/Timer.h>
#include <stdio.h>
#include <timepix3/Timepix3Instrument.h>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

const char *classname = "Timepix3 detector with ESS readout";

Timepix3Base::Timepix3Base(BaseSettings const &settings) : Detector(settings) {
  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets", ITCounters.RxPackets);
  Stats.create("receive.bytes", ITCounters.RxBytes);
  Stats.create("receive.dropped", ITCounters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);

 
  Stats.create("readouts.pixel_readout_count", Counters.PixelReadouts);
  Stats.create("readouts.tdc_readout_count", Counters.TDCReadouts);
  Stats.create("readouts.tdc1rising_readout_count", Counters.TDC1RisingReadouts);
  Stats.create("readouts.tdc1falling_readout_count", Counters.TDC1FallingReadouts);
  Stats.create("readouts.tdc2rising_readout_count", Counters.TDC2RisingReadouts);
  Stats.create("readouts.tdc2falling_readout_count", Counters.TDC2FallingReadouts);
  Stats.create("readouts.globaltimestamp_readout_count", Counters.GlobalTimestampReadouts);
  Stats.create("readouts.evrtimestamp_readout_count", Counters.EVRTimestampReadouts);
  Stats.create("readouts.undefined_readout_count", Counters.UndefinedReadouts);
  Stats.create("readouts.tof_count", Counters.TofCount);
  Stats.create("readouts.tof_neg", Counters.TofNegative);
  Stats.create("readouts.prevtof_count", Counters.PrevTofCount);
  Stats.create("readouts.prevtof_neg", Counters.PrevTofNegative);
  Stats.create("readouts.tof_high", Counters.TofHigh);
  Stats.create("readouts.prevtof_high", Counters.PrevTofHigh);

  // Events
  Stats.create("events.count", Counters.Events);
  Stats.create("events.pixel_errors", Counters.PixelErrors);

  // System counters
  Stats.create("thread.input_idle", ITCounters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);


  Stats.create("transmit.bytes", Counters.TxBytes);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", Counters.kafka_produce_fails);
  Stats.create("kafka.ev_errors", Counters.kafka_ev_errors);
  Stats.create("kafka.ev_others", Counters.kafka_ev_others);
  Stats.create("kafka.dr_errors", Counters.kafka_dr_errors);
  Stats.create("kafka.dr_others", Counters.kafka_dr_noerrors);
  // clang-format on
  std::function<void()> inputFunc = [this]() { inputThread(); };
  AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    Timepix3Base::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Timepix3 Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

///
/// \brief Normal processing thread
void Timepix3Base::processingThread() {
  if (EFUSettings.KafkaTopic == "") {
    XTRACE(INIT, ERR, "No kafka topic set, using DetectorName + _detector");
    EFUSettings.KafkaTopic = EFUSettings.DetectorName + "_detector";
  }

  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                         KafkaCfg.CfgParms);

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  Serializer = new EV44Serializer(KafkaBufferSize, "timepix3", Produce);
  Timepix3Instrument Timepix3(Counters, EFUSettings);
  Timepix3.setSerializer(
      Serializer); // would rather have this in Timepix3Instrument

  unsigned int DataIndex;
  TSCTimer ProduceTimer(EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ);

  RuntimeStat RtStat({ITCounters.RxPackets, Counters.Events, Counters.TxBytes});

  while (runThreads) {
    if (InputFifo.pop(DataIndex)) { // There is data in the FIFO - do processing
      auto DataLen = RxRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }
      XTRACE(DATA, DEB, "getting data buffer");
      /// \todo use the Buffer<T> class here and in parser?
      /// \todo avoid copying by passing reference to stats like for gdgem?
      auto DataPtr = RxRingbuffer.getDataBuffer(DataIndex);

      XTRACE(DATA, DEB, "parsing data");
      // parse readout data
      Timepix3.Timepix3Parser.parse(DataPtr, DataLen);

      XTRACE(DATA, DEB, "processing data");

      // Process readouts, generate (end produce) events
      Timepix3.processReadouts();

    } else { // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    if (ProduceTimer.timeout()) {
      // XTRACE(DATA, DEB, "Serializer timer timed out, producing message now");
      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {ITCounters.RxPackets, Counters.Events, Counters.TxBytes});

      Serializer->produce();
    }
    /// Kafka stats update - common to all detectors
    /// don't increment as Producer & Serializer keep absolute count
    Counters.kafka_produce_fails = EventProducer.stats.produce_fails;
    Counters.kafka_ev_errors = EventProducer.stats.ev_errors;
    Counters.kafka_ev_others = EventProducer.stats.ev_others;
    Counters.kafka_dr_errors = EventProducer.stats.dr_errors;
    Counters.kafka_dr_noerrors = EventProducer.stats.dr_noerrors;
    Counters.TxBytes = Serializer->TxBytes;
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}
} // namespace Timepix3
