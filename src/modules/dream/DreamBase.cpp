// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of the detector pipeline plugin for DREAM
/// \todo unofficial - not reviewed Readout structure
//===----------------------------------------------------------------------===//

#include <modules/dream/DreamBase.h>
#include <modules/dream/DreamInstrument.h>

#include <common/RuntimeStat.h>
#include <common/debug/Trace.h>
#include <common/detector/EFUArgs.h>
#include <common/kafka/KafkaConfig.h>
#include <unistd.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Dream {

const char *classname = "DREAM detector with ESS readout";

DreamBase::DreamBase(BaseSettings const &Settings, DetectorType type)
    : Detector(Settings), Type(type) {

  XTRACE(INIT, ALW, "Adding stats");

  // clang-format off

  // ESS Readout
  Stats.create("essheader.error_header", Counters.ErrorESSHeaders);

  // ESS Readout Data Header
  Stats.create("readouts.count", Counters.Readouts);
  Stats.create("readouts.headers", Counters.DataHeaders);
  Stats.create("readouts.error_buffer", Counters.BufferErrors);
  Stats.create("readouts.error_datalen", Counters.DataLenErrors);
  Stats.create("readouts.error_fiber", Counters.FiberErrors);
  Stats.create("readouts.error_fen", Counters.FENErrors);

  //
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  Stats.create("events.count", Counters.Events);

  // Produce cause call stats
  Stats.create("produce.cause.timeout", Counters.ProduceCauseTimeout);
  // clang-format on

  std::function<void()> inputFunc = [this]() { inputThread(); };
  AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    DreamBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Dream Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

///
/// \brief Normal processing thread
void DreamBase::processingThread() {

  KafkaConfig KafkaCfg(EFUSettings.KafkaConfigFile);

  Producer EventProducer(EFUSettings.KafkaBroker, EFUSettings.KafkaTopic,
                         KafkaCfg.CfgParms, Stats);

  auto Produce = [&EventProducer](const auto &DataBuffer,
                                  const auto &Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  Serializer = std::make_unique<EV44Serializer>(
      KafkaBufferSize, EFUSettings.DetectorName, Produce);

  Stats.create("produce.cause.pulse_change",
               Serializer->stats().ProduceRefTimeTriggered);
  Stats.create("produce.cause.max_events_reached",
               Serializer->stats().ProduceTriggeredMaxEvents);

  DreamInstrument Dream(Stats, Counters, EFUSettings, *Serializer,
                        ESSHeaderParser);

  unsigned int DataIndex;
  Timer ProduceTimer;

  RuntimeStat RtStat({getInputCounters().RxPackets, Counters.Events,
                      EventProducer.getStats().MsgStatusPersisted});

  while (runThreads) {

    auto idle_start = esstime::local_clock::now();

    if (InputFifo.pop(DataIndex)) { // There is data in the FIFO - do processing
      auto DataLen = RxRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        ITCounters.FifoSeqErrors++;
        continue;
      }

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
      Res = Dream.DreamParser.parse(ESSHeaderParser.Packet.DataPtr,
                                    ESSHeaderParser.Packet.DataLength);

      // Process readouts, generate (end produce) events
      Dream.processReadouts();

    } else { // There is NO data in the FIFO - do stop checks and sleep a little
      usleep(100);
      Counters.ProcessingIdle +=
          std::chrono::duration_cast<std::chrono::microseconds>(
              esstime::local_clock::now() - idle_start)
              .count();
    }

    /// poll producer stats
    EventProducer.poll(0);

    // Time out after one second
    if (ProduceTimer.timeNS() >=
        EFUSettings.UpdateIntervalSec * 1'000'000'000) {

      RuntimeStatusMask = RtStat.getRuntimeStatusMask(
          {getInputCounters().RxPackets, Counters.Events,
           EventProducer.getStats().MsgStatusPersisted});

      Serializer->produce();
      Counters.ProduceCauseTimeout++;

      ProduceTimer.reset();
    }
  }
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}
} // namespace Dream
