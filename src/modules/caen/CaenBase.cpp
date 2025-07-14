// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of the detector pipeline plugin for Caen
/// detectors
//===----------------------------------------------------------------------===//

#include <common/RuntimeStat.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/KafkaConfig.h>
#include <common/time/TimeString.h>
#include <common/time/Timer.h>
#include <memory>
#include <modules/caen/CaenBase.h>
#include <modules/caen/CaenInstrument.h>

#include <cinttypes>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

const char *classname = "Caen detector with ESS readout";

CaenBase::CaenBase(BaseSettings const &settings, DetectorType type)
    : Detector(settings), Type(type) {
      
  XTRACE(INIT, ALW, "Adding stats");
  
  // clang-format off
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);

  // LoKI Readout Data
  Stats.create("parser.readout.header.count", Counters.Parser.DataHeaders);
  Stats.create("parser.readout.count", Counters.Parser.Readouts);
  Stats.create("parser.readout.error_maxadc", Counters.Parser.ReadoutsMaxADC);
  Stats.create("parser.readout.error_headersize", Counters.Parser.DataHeaderSizeErrors);
  Stats.create("parser.readout.error_datlen_mismatch", Counters.Parser.DataLenMismatch);
  Stats.create("parser.readout.error_datlen_invalid", Counters.Parser.DataLenInvalid);
  Stats.create("parser.readout.error_ringfen", Counters.Parser.RingFenErrors);

  // Logical and Digital geometry incl. Calibration
  Stats.create("geometry.ring_errors", Counters.Geom.RingErrors);
  Stats.create("geometry.fen_errors", Counters.Geom.FENErrors);
  Stats.create("geometry.ring_mapping_errors", Counters.Geom.RingMappingErrors);
  Stats.create("geometry.fen_mapping_errors", Counters.Geom.FENMappingErrors);
  Stats.create("geometry.topology_errors", Counters.Geom.TopologyErrors);
  Stats.create("geometry.group_errors", Counters.Geom.GroupErrors);
  Stats.create("geometry.ampl_zero", Counters.Geom.AmplitudeZero);
  Stats.create("geometry.ampl_low", Counters.Geom.AmplitudeLow);
  Stats.create("geometry.ampl_high", Counters.Geom.AmplitudeHigh);
  Stats.create("geometry.pos_low", Counters.Calibration.ClampLow);
  Stats.create("geometry.pos_high", Counters.Calibration.ClampHigh);
  Stats.create("geometry.calib_group_errors", Counters.Calibration.GroupErrors);
  Stats.create("geometry.outside_tube", Counters.Calibration.OutsideInterval);

  // Events
  Stats.create("events.count", Counters.Events);
  Stats.create("events.errors.pixel", Counters.PixelErrors);
  Stats.create("events.errors.time", Counters.TimeError);

  // Monitor and calibration stats
  Stats.create("transmit.monitor_packets", Counters.TxRawReadoutPackets);

  // System counters
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  // Produce cause call stats
  Stats.create("produce.cause.timeout", Counters.ProduceCauseTimeout);

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
                         KafkaCfg.CfgParms, &Stats);

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  // Create the instrument
  CaenInstrument Caen(Counters, EFUSettings, ESSHeaderParser);
  // and its serializers
  Serializers.reserve(Caen.Geom->numSerializers());
  for (size_t i = 0; i < Caen.Geom->numSerializers(); ++i) {
    Serializers.emplace_back(std::make_shared<EV44Serializer>(
        KafkaBufferSize, Caen.Geom->serializerName(i), Produce));
  }
  // give the instrument shared pointers to the serializers
  Caen.setSerializers(Serializers);

  // Create the raw-data monitor producer and serializer
  Producer MonitorProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaDebugTopic,
                           KafkaCfg.CfgParms);

  auto ProduceMonitor = [&MonitorProducer](const auto &DataBuffer,
                                           const auto &Timestamp) {
    MonitorProducer.produce(DataBuffer, Timestamp);
  };

  MonitorSerializer = std::make_unique<AR51Serializer>(EFUSettings.DetectorName,
                                                       ProduceMonitor);

  RuntimeStat RtStat({getInputCounters().RxPackets, Counters.Events,
                      EventProducer.getStats().MsgStatusPersisted});

  // Time out after one second
  Timer ProduceTimer(EFUSettings.UpdateIntervalSec * 1'000'000'000);

  unsigned int DataIndex;
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
      auto Res = ESSHeaderParser.validate(DataPtr, DataLen, Type);

      if (Res != ESSReadout::Parser::OK) {
        XTRACE(DATA, DEB, "Error parsing ESS readout header");
        Counters.ErrorESSHeaders++;
        continue;
      }

      // We have good header information, now parse readout data
      Res = Caen.CaenParser.parse(ESSHeaderParser.Packet.DataPtr,
                                  ESSHeaderParser.Packet.DataLength);

      // Process readouts, generate (and produce) events
      Caen.processReadouts();

      // send monitoring data
      if (getInputCounters().RxPackets % EFUSettings.MonitorPeriod <
          EFUSettings.MonitorSamples) {
        XTRACE(PROCESS, DEB, "Serialize and stream monitor data for packet %lu",
               getInputCounters().RxPackets);
        MonitorSerializer->serialize((uint8_t *)DataPtr, DataLen);
        MonitorSerializer->produce();
        Counters.TxRawReadoutPackets++;
      }

      /// \todo This could be moved and done less frequently
      Counters.Parser = Caen.CaenParser.Stats;
      Counters.Geom = Caen.Geom->Stats;
      Counters.Calibration = Caen.Geom->CaenCDCalibration.Stats;

    } else { // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.ProcessingIdle++;
      usleep(10);
    }

    EventProducer.poll(0);

    if (ProduceTimer.timeout()) {
      // XTRACE(DATA, DEB, "Serializer timer timed out, producing message now");
      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {getInputCounters().RxPackets, Counters.Events,
           EventProducer.getStats().MsgStatusPersisted});
      // Produce messages if there are events
      for (auto &Serializer : Serializers) {
        Serializer->produce();
      }

      // update counter statistics
      Counters.ProduceCauseTimeout++;
      Counters.ProduceCausePulseChange = std::transform_reduce(
          Serializers.begin(), Serializers.end(), 0, std::plus<>(),
          [](auto &Serializer) { return Serializer->ProduceCausePulseChange; });
      Counters.ProduceCauseMaxEventsReached = std::transform_reduce(
          Serializers.begin(), Serializers.end(), 0, std::plus<>(),
          [](auto &Serializer) {
            return Serializer->ProduceCauseMaxEventsReached;
          });
    }
  }

  XTRACE(INPUT, ALW, "Stopping processing thread.");
}
} // namespace Caen
