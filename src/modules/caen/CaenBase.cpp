// Copyright (C) 2019-2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of the detector pipeline plugin for Caen
/// detectors
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <common/RuntimeStat.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/KafkaConfig.h>
#include <common/time/TimeString.h>
#include <common/time/Timer.h>
#include <modules/caen/CaenBase.h>
#include <modules/caen/CaenInstrument.h>
#include <unistd.h>
#include <common/time/CheckTimer.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

const char *classname = "Caen detector with ESS readout";

CaenBase::CaenBase(BaseSettings const &settings,
                   ESSReadout::Parser::DetectorType t)
    : Detector(settings), type(t) {
  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets", ITCounters.RxPackets);
  Stats.create("receive.bytes", ITCounters.RxBytes);
  Stats.create("receive.dropped", ITCounters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);
  Stats.create("transmit.monitor_packets", Counters.TxRawReadoutPackets);

  // ESS Readout
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
  Stats.create("essheader.version.v0", Counters.ReadoutStats.Version0Header);
  Stats.create("essheader.version.v1", Counters.ReadoutStats.Version1Header);

  for (int i = 0; i < 12; i++) {
    std::string statname = fmt::format("essheader.OQ.{:02}.packets", i);
    Stats.create(statname, Counters.ReadoutStats.OQRxPackets[i]);
  }

  // LoKI Readout Data
  Stats.create("readouts.headers", Counters.Parser.DataHeaders);
  Stats.create("readouts.count", Counters.Parser.Readouts);
  Stats.create("readouts.error_amplitude", Counters.Parser.ReadoutsBadAmpl);
  Stats.create("readouts.error_headersize", Counters.Parser.DataHeaderSizeErrors);
  Stats.create("readouts.error_datlen_mismatch", Counters.Parser.DataLenMismatch);
  Stats.create("readouts.error_datlen_invalid", Counters.Parser.DataLenInvalid);
  Stats.create("readouts.error_ringfen", Counters.Parser.RingFenErrors);

  Stats.create("readouts.tof_count", Counters.TimeStats.TofCount);
  Stats.create("readouts.tof_neg", Counters.TimeStats.TofNegative);
  Stats.create("readouts.prevtof_count", Counters.TimeStats.PrevTofCount);
  Stats.create("readouts.prevtof_neg", Counters.TimeStats.PrevTofNegative);
  Stats.create("readouts.tof_high", Counters.TimeStats.TofHigh);
  Stats.create("readouts.prevtof_high", Counters.TimeStats.PrevTofHigh);

  // Logical and Digital geometry incl. Calibration
  Stats.create("geometry.ring_mapping_errors", Counters.Geom.RingErrors);
  Stats.create("geometry.fen_mapping_errors", Counters.Geom.FENErrors);
  Stats.create("geometry.group_errors", Counters.Geom.GroupErrors);
  Stats.create("geometry.ampl_zero", Counters.Geom.AmplitudeZero);
  Stats.create("geometry.pos_low", Counters.Calibration.ClampLow);
  Stats.create("geometry.pos_high", Counters.Calibration.ClampHigh);
  Stats.create("geometry.calib_group_errors", Counters.Calibration.GroupErrors);
  Stats.create("geometry.outside_tube", Counters.Calibration.OutsideInterval);

  // Events
  Stats.create("events.count", Counters.Events);
  Stats.create("events.pixel_errors", Counters.PixelErrors);

  Stats.create("transmit.calibmode_packets", ITCounters.CalibModePackets);

  // System counters
  Stats.create("thread.input_idle", ITCounters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  // Produce cause call stats
  Stats.create("produce.cause.timeout", Counters.ProduceCauseTimeout);

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
    CaenBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Caen Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

///
/// \brief Normal processing thread
void CaenBase::processingThread() {
  if (EFUSettings.KafkaTopic.empty()) {
    XTRACE(INIT, ERR, "No kafka topic set, using DetectorName + _detector");
    EFUSettings.KafkaTopic = EFUSettings.DetectorName + "_detector";
  }

  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);
  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                         KafkaCfg.CfgParms);

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  // Create the instrument
  CaenInstrument Caen(Counters, EFUSettings);
  // and its serializers
  Serializers.reserve(Caen.Geom->numSerializers());
  for (size_t i=0; i < Caen.Geom->numSerializers(); ++i){
    Serializers.emplace_back(std::make_shared<EV44Serializer>(KafkaBufferSize, Caen.Geom->serializerName(i), Produce));
  }
  // give the instrument shared pointers to the serializers
  Caen.setSerializers(Serializers);

  // Create the raw-data monitor producer and serializer
  Producer MonitorProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaDebugTopic,
                           KafkaCfg.CfgParms);
  auto ProduceMonitor = [&MonitorProducer](auto DataBuffer, auto Timestamp) {
    MonitorProducer.produce(DataBuffer, Timestamp);
  };

  MonitorSerializer = new AR51Serializer(EFUSettings.DetectorName, ProduceMonitor);

  unsigned int DataIndex;

  RuntimeStat RtStat({ITCounters.RxPackets, Counters.Events,
                      Counters.KafkaStats.produce_bytes_ok});
  // Create the periodic timer for producing messages, in case of low event rate
  // TSCTimer ProduceTimer(EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ); // x86 specific
  CheckTimer ProduceTimer(EFUSettings.UpdateIntervalSec * 1'000'000'000);

  while (runThreads) {
    if (InputFifo.pop(DataIndex)) { // There is data in the FIFO - do processing
      auto DataLen = RxRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        XTRACE(DATA, ERR, "Data length in FIFO is zero");
        Counters.FifoSeqErrors++;
        continue;
      }

      XTRACE(DATA, DEB, "Ringbuffer index %d has data of length %d", DataIndex,
             DataLen);

      /// \todo use the Buffer<T> class here and in parser?
      /// \todo avoid copying by passing reference to stats like for gdgem?
      auto DataPtr = RxRingbuffer.getDataBuffer(DataIndex);
      auto Res = Caen.ESSReadoutParser.validate(DataPtr, DataLen, type);

      /// \todo could be moved
      Counters.ReadoutStats = Caen.ESSReadoutParser.Stats;

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, DEB, "Error parsing ESS readout header");
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = Caen.CaenParser.parse(Caen.ESSReadoutParser.Packet.DataPtr,
                                  Caen.ESSReadoutParser.Packet.DataLength);

      // Process readouts, generate (and produce) events
      Caen.processReadouts();

      // send monitoring data
      if (ITCounters.RxPackets % EFUSettings.MonitorPeriod <
          EFUSettings.MonitorSamples) {
        XTRACE(PROCESS, DEB, "Serialize and stream monitor data for packet %lu",
               ITCounters.RxPackets);
        MonitorSerializer->serialize((uint8_t *)DataPtr, DataLen);
        MonitorSerializer->produce();
        Counters.TxRawReadoutPackets++;
      }

      /// \todo This could be moved and done less frequently
      Counters.Parser = Caen.CaenParser.Stats;
      Counters.TimeStats = Caen.ESSReadoutParser.Packet.Time.Stats;
      Counters.Geom = Caen.Geom->Stats;
      Counters.Calibration = Caen.Geom->CaenCDCalibration.Stats;

    } else { // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    if (ProduceTimer.timeout()){
      // XTRACE(DATA, DEB, "Serializer timer timed out, producing message now");
      RuntimeStatusMask =
          RtStat.getRuntimeStatusMask({ITCounters.RxPackets, Counters.Events,
                                       Counters.KafkaStats.produce_bytes_ok});
      // Produce messages if there are events
      for (auto &Serializer: Serializers) {
        Serializer->produce();
      }

      // update counter statistics
      Counters.ProduceCauseTimeout++;
      Counters.ProduceCausePulseChange = std::transform_reduce(
          Serializers.begin(), Serializers.end(), 0, std::plus<>(),
          [](auto &Serializer) { return Serializer->ProduceCausePulseChange; }
      );
      Counters.ProduceCauseMaxEventsReached = std::transform_reduce(
          Serializers.begin(), Serializers.end(), 0, std::plus<>(),
          [](auto &Serializer) { return Serializer->ProduceCauseMaxEventsReached; }
      );
    }

    /// Kafka stats update - common to all detectors
    /// don't increment as Producer & Serializer keep absolute count
    Counters.KafkaStats = EventProducer.stats;
  }

  XTRACE(INPUT, ALW, "Stopping processing thread.");
}
} // namespace Caen
