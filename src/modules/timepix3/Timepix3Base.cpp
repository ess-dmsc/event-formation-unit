// Copyright (C) 2023 - 2025 European Spallation Source, see LICENSE file
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
#include <common/memory/span.hpp>
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

  // Counters related to timing event handling and time synchronization
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
  Stats.create("thread.stages.parsing.process_us", Counters.Stage1ProcessingTimeUs);
  Stats.create("thread.stages.parsing.starving_us", Counters.Stage1StarvingTimeUs);
  Stats.create("thread.stages.parsing.blocked_us", Counters.Stage1BlockedTimeUs);

  Stats.create("thread.stages.sorting.process_us", Counters.Stage2ProcessingTimeUs);
  Stats.create("thread.stages.sorting.starving_us", Counters.Stage2StarvingTimeUs);
  Stats.create("thread.stages.sorting.blocked_us", Counters.Stage2BlockedTimeUs);

  Stats.create("thread.stages.future.process_us", Counters.Stage3ProcessingTimeUs);
  Stats.create("thread.stages.future.starving_us", Counters.Stage3StarvingTimeUs);
  Stats.create("thread.stages.future.blocked_us", Counters.Stage3BlockedTimeUs);

  Stats.create("thread.stages.cluster.process_us", Counters.Stage4ProcessingTimeUs);
  Stats.create("thread.stages.cluster.starving_us", Counters.Stage4StarvingTimeUs);
  Stats.create("thread.stages.cluster.blocked_us", Counters.Stage4BlockedTimeUs);

  Stats.create("thread.stages.publish.process_us", Counters.Stage5ProcessingTimeUs);
  Stats.create("thread.stages.publish.starving_us", Counters.Stage5StarvingTimeUs);
  Stats.create("thread.stages.publish.blocked_us", Counters.Stage5BlockedTimeUs);

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
                         KafkaCfg.CfgParms, &Stats);

  auto Produce = [&EventProducer](const auto &DataBuffer,
                                  const auto &Timestamp) {
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
    EventProducer.getStats().MsgStatusPersisted});

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

  static std::vector<uint64_t> CombinedData;

  int AddExtraPackets = 0;
  bool BufferFlush = false;

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
        /// EVR data found trigger buffer flush
        BufferFlush = true;
        Timepix3.timepix3Parser.parseEVR(DataPtr);
        continue;
      }

      // Copy the data to a vector for the pipeline
      nonstd::span<uint64_t> ReadoutData(reinterpret_cast<uint64_t*>(DataPtr), DataLen / sizeof(uint64_t));
      CombinedData.insert(CombinedData.end(), ReadoutData.begin(), ReadoutData.end());

      /// If EVR found we add some more packets and hand over the data for the pipeline to process
      /// \todo this is unheatly for timing point of view since it delays the tdc packet calculation
      if (BufferFlush) {
        AddExtraPackets++;

      // if EVR found add 3 extra packet to the buffer and then hand over the data for the pipeline to process
        if (AddExtraPackets >= 3) {
            auto InputQueue = Timepix3.DataPipeline.getInputQueue<std::vector<uint64_t>>();

            // learn the size of the vector to reuse it for reallocation
            auto CombinedDataSize = CombinedData.size();

            // Enqueue the combined data to the pipeline
            while (!InputQueue->enqueue(std::move(CombinedData))) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }

            // Reset counter buffer and flags for the next pulse cycle
            AddExtraPackets = 0;
            BufferFlush = false;
            // The vector moved to the pipeline, so we need clear and reset size to reinitiate the vector
            CombinedData.clear();
            CombinedData.reserve(CombinedDataSize);
        }
      }

    } else { // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.ProcessingIdle++;
      this_thread::sleep_for(std::chrono::microseconds(1));
    }

  
    EventProducer.poll(0);
    
    Counters.Stage1ProcessingTimeUs =
        Timepix3.DataPipeline.getStagePerformanceUs(0);
    Counters.Stage1StarvingTimeUs =
        Timepix3.DataPipeline.getStageStarvingCounterUs(0);
    Counters.Stage1BlockedTimeUs =
        Timepix3.DataPipeline.getStageBlockedCounterUs(0);

    Counters.Stage2ProcessingTimeUs =
        Timepix3.DataPipeline.getStagePerformanceUs(1);
    Counters.Stage2StarvingTimeUs =
        Timepix3.DataPipeline.getStageStarvingCounterUs(1);
    Counters.Stage2BlockedTimeUs =
        Timepix3.DataPipeline.getStageBlockedCounterUs(1);

    Counters.Stage3ProcessingTimeUs =
        Timepix3.DataPipeline.getStagePerformanceUs(2);
    Counters.Stage3StarvingTimeUs =
        Timepix3.DataPipeline.getStageStarvingCounterUs(2);
    Counters.Stage3BlockedTimeUs =
        Timepix3.DataPipeline.getStageBlockedCounterUs(2);

    Counters.Stage4ProcessingTimeUs =
        Timepix3.DataPipeline.getStagePerformanceUs(3);
    Counters.Stage4StarvingTimeUs =
        Timepix3.DataPipeline.getStageStarvingCounterUs(3);
    Counters.Stage4BlockedTimeUs =
        Timepix3.DataPipeline.getStageBlockedCounterUs(3);

    Counters.Stage5ProcessingTimeUs =
        Timepix3.DataPipeline.getStagePerformanceUs(4);
    Counters.Stage5StarvingTimeUs =
        Timepix3.DataPipeline.getStageStarvingCounterUs(4);
    Counters.Stage5BlockedTimeUs =
        Timepix3.DataPipeline.getStageBlockedCounterUs(4);

    if (ProduceTimer.timeout()) {
      // XTRACE(DATA, DEB, "Serializer timer timed out, producing message now");
      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {ITCounters.RxPackets, Counters.Events,
           EventProducer.getStats().MsgStatusPersisted});
      Serializer.produce();
      Counters.ProduceCauseTimeout++;
    }
  }

  dequeueThread.wait();

  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}
} // namespace Timepix3
