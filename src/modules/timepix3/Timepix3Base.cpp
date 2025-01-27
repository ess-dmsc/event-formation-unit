// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of the detector pipeline plugin for Timepix3
/// detectors
//===----------------------------------------------------------------------===//

#include <chrono>
#include <common/RuntimeStat.h>
#include <common/detector/BaseSettings.h>
#include <common/kafka/KafkaConfig.h>
#include <cstdint>
#include <memory>
#include <modules/timepix3/Timepix3Base.h>
#include <modules/timepix3/Timepix3Instrument.h>
#include <thread>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

const char *classname = "Timepix3 detector with ESS readout";

Timepix3Base::Timepix3Base(BaseSettings const &settings)
    : Detector(settings), timepix3Configuration(settings.ConfigFile) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets", ITCounters.RxPackets);
  Stats.create("receive.bytes", ITCounters.RxBytes);
  Stats.create("receive.dropped", ITCounters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);
 
  // Counters related to readouts
  Stats.create("readouts.pixel_readout_count", Counters.PixelReadouts);
  Stats.create("readouts.parsing_us", Counters.ReadoutParsingUs);
  Stats.create("readouts.tdc.tdc1rising_readout_count", Counters.TDC1RisingReadouts);
  Stats.create("readouts.tdc.tdc1falling_readout_count", Counters.TDC1FallingReadouts);
  Stats.create("readouts.tdc.tdc2rising_readout_count", Counters.TDC2RisingReadouts);
  Stats.create("readouts.tdc.unknown_tdc_type_count", Counters.UnknownTDCReadouts);
  Stats.create("readouts.tdc.tdc2falling_readout_count", Counters.TDC2FallingReadouts);
  Stats.create("readouts.evr.evr_readout_count", Counters.EVRReadoutCounter);
  Stats.create("readouts.evr.evr_processing_us", Counters.EVRProcessingTimeUs);
  Stats.create("readouts.evr.evr_readout_dropped", Counters.EVRReadoutDropped);
  Stats.create("readouts.tdc.tdc_readout_count", Counters.TDCReadoutCounter);
  Stats.create("readouts.tdc.tdc_readout_dropped", Counters.TDCReadoutDropped);
  Stats.create("readouts.tdc.tdc_processing_us", Counters.TDCProcessingTimeUs);
  Stats.create("readouts.undefined_readout_count", Counters.UndefinedReadoutCounter);

  // Counters related to timing event handling and time syncronization
  Stats.create("handlers.timeingevent.miss_tdc_count", Counters.MissTDCCounter);
  Stats.create("handlers.timeingevent.miss_evr_count", Counters.MissEVRCounter);
  Stats.create("handlers.timeingevent.evr_pair_count", Counters.EVRPairFound);
  Stats.create("handlers.timeingevent.tdc_pair_count", Counters.TDCPairFound);
  Stats.create("handlers.timeingevent.ess_global_time_count", Counters.ESSGlobalTimeCounter);

  // Counters related to pixel event handling and event calculation
  Stats.create("handlers.pixelevent.no_global_time_error", Counters.NoGlobalTime);
  Stats.create("handlers.pixelevent.invalid_pixel_readout", Counters.InvalidPixelReadout);
  Stats.create("handlers.pixelevent.event_time_next_pulse_count", Counters.EventTimeForNextPulse);
  Stats.create("handlers.pixelevent.tof_count", Counters.TofCount);
  Stats.create("handlers.pixelevent.tof_neg", Counters.TofNegative);
  Stats.create("handlers.pixelevent.cluster_size_to_small", Counters.ClusterSizeTooSmall);
  Stats.create("handlers.pixelevent.cluster_to_short", Counters.ClusterToShort);
  Stats.create("handlers.pixelevent.prevtof_count", Counters.PrevTofCount);

  // Events
  Stats.create("events.count", Counters.Events);
  Stats.create("events.pixel_errors", Counters.PixelErrors);

  // System counters
  Stats.create("thread.input_idle", ITCounters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  // Produce cause call stats
  Stats.create("produce.cause.timeout", Counters.ProduceCauseTimeout);

  // Time measurement on stages
  Stats.create("thread.stages.parsing_us", Counters.Stage1ProcessingTimeUs);
  Stats.create("thread.stages.sorting_us", Counters.Stage2ProcessingTimeUs);
  Stats.create("thread.stages.cluster_us", Counters.Stage3ProcessingTimeUs);
  Stats.create("thread.stages.publish_us", Counters.Stage4ProcessingTimeUs);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.config_errors", Counters.KafkaStats.config_errors);
  Stats.create("kafka.produce_bytes_ok", Counters.KafkaStats.produce_bytes_ok);
  Stats.create("kafka.produce_bytes_error", Counters.KafkaStats.produce_bytes_error);
  Stats.create("kafka.produce_calls", Counters.KafkaStats.produce_calls);
  Stats.create("kafka.produce_no_errors", Counters.KafkaStats.produce_no_errors);
  Stats.create("kafka.produce_errors", Counters.KafkaStats.produce_errors);
  Stats.create("kafka.err_unknown_topic", Counters.KafkaStats.err_unknown_topic);
  Stats.create("kafka.err_queue_full", Counters.KafkaStats.err_queue_full);
  Stats.create("kafka.err_other", Counters.KafkaStats.err_other);
  Stats.create("kafka.ev_errors", Counters.KafkaStats.ev_errors);
  Stats.create("kafka.ev_others", Counters.KafkaStats.ev_others);
  Stats.create("kafka.dr_errors", Counters.KafkaStats.dr_errors);
  Stats.create("kafka.dr_others", Counters.KafkaStats.dr_noerrors);
  Stats.create("kafka.librdkafka_msg_cnt", Counters.KafkaStats.librdkafka_msg_cnt);
  Stats.create("kafka.librdkafka_msg_size", Counters.KafkaStats.librdkafka_msg_size);

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

/// Counters
///  \brief Normal processing thread
void Timepix3Base::processingThread() {
  if (EFUSettings.KafkaTopic == "") {
    XTRACE(INIT, ERR, "No kafka topic set, using DetectorName + _detector");
    EFUSettings.KafkaTopic = EFUSettings.DetectorName + "_detector";
  }

  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                         KafkaCfg.CfgParms);

  auto Produce = [&EventProducer](auto &DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  EV44Serializer Serializer(KafkaBufferSize, "timepix3", Produce);

  Stats.create("produce.cause.pulse_change",
               Serializer.stats().ProduceRefTimeTriggered);
  Stats.create("produce.cause.max_events_reached",
               Serializer.stats().ProduceTriggeredMaxEvents);

  Timepix3Instrument Timepix3(Counters, timepix3Configuration, Serializer);

  unsigned int DataIndex;
  TSCTimer ProduceTimer(EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ);

  RuntimeStat RtStat({ITCounters.RxPackets, Counters.Events,
                      Counters.KafkaStats.produce_bytes_ok});

  auto OutputQueue = Timepix3.DataPipeline.getOutputQueue<int>();
  std::shared_ptr<std::atomic_bool> run =
      std::make_shared<std::atomic_bool>(true);

  std::future<void> dequeueThread =
      std::async(std::launch::async, [OutputQueue, run = &runThreads]() {
        int output = false;
        while (run->load()) {
          if (!OutputQueue->dequeue(output)) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
          }
        }
      });

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

      if (DataLen == 24) {
        Timepix3.timepix3Parser.parseEVR(DataPtr);
        continue;
      }

      // Copy the data to a vector for the pipeline
      std::vector<uint64_t> ReadoutData(DataLen / sizeof(uint64_t));
      ReadoutData.insert(ReadoutData.begin(),
                         reinterpret_cast<uint64_t *>(DataPtr),
                         reinterpret_cast<uint64_t *>(DataPtr + DataLen));

      auto InputQueue =
          Timepix3.DataPipeline.getInputQueue<std::vector<uint64_t>>();

      while (!InputQueue->enqueue(std::move(ReadoutData))) {
        std::this_thread::sleep_for(chrono::microseconds(1));
      }

    } else { // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.ProcessingIdle++;
      this_thread::sleep_for(std::chrono::microseconds(1));
    }

    Counters.Stage1ProcessingTimeUs =
        Timepix3.DataPipeline.getStagePerformance(0);
    Counters.Stage2ProcessingTimeUs =
        Timepix3.DataPipeline.getStagePerformance(1);
    Counters.Stage3ProcessingTimeUs =
        Timepix3.DataPipeline.getStagePerformance(2);
    Counters.Stage4ProcessingTimeUs =
        Timepix3.DataPipeline.getStagePerformance(3);

    if (ProduceTimer.timeout()) {
      // XTRACE(DATA, DEB, "Serializer timer timed out, producing message now");
      RuntimeStatusMask =
          RtStat.getRuntimeStatusMask({ITCounters.RxPackets, Counters.Events,
                                       Counters.KafkaStats.produce_bytes_ok});
      Serializer.produce();
      Counters.ProduceCauseTimeout++;
      Counters.KafkaStats = EventProducer.stats;
    }
  }

  dequeueThread.wait();

  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}
} // namespace Timepix3
