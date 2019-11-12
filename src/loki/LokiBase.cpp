/** Copyright (C) 2019 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of the detector pipeline plugin for Loki
/// detectors.
//===----------------------------------------------------------------------===//

#include "LokiBase.h"

#include <cinttypes>
#include <common/EFUArgs.h>
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/monitor/HistogramSerializer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <common/TimeString.h>
#include <common/TestImageUdder.h>

#include <unistd.h>

#include <common/SPSCFifo.h>
#include <common/Socket.h>
#include <common/TSCTimer.h>
#include <common/Timer.h>

#include <loki/readout/DataParser.h>
#include <readout/ESSTime.h>
#include <loki/geometry/Geometry.h>
#include <loki/geometry/HeliumTube.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Loki {

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "Loki detector with ESS readout";

LokiBase::LokiBase(BaseSettings const &Settings, struct LokiSettings &LocalLokiSettings)
    : Detector("Loki", Settings), LokiModuleSettings(LocalLokiSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("receive.packets", Counters.RxPackets);
  Stats.create("receive.bytes", Counters.RxBytes);
  Stats.create("receive.dropped", Counters.FifoPushErrors);
  Stats.create("receive.fifo_seq_errors", Counters.FifoSeqErrors);

  // ESS Readout
  Stats.create("readouts.error_buffer", Counters.ErrorBuffer);
  Stats.create("readouts.error_size", Counters.ErrorSize);
  Stats.create("readouts.error_version", Counters.ErrorVersion);
  Stats.create("readouts.error_type", Counters.ErrorTypeSubType);
  Stats.create("readouts.error_seqno", Counters.ErrorSeqNum);
  // LoKI Readout Data
  Stats.create("readouts.count", Counters.Readouts);
  Stats.create("readouts.headers", Counters.Headers);
  Stats.create("readouts.error_bytes", Counters.ErrorBytes);
  Stats.create("readouts.error_header", Counters.ErrorHeaders);

  //
  Stats.create("thread.processing_idle", Counters.RxIdle);

  Stats.create("events.count", Counters.Events);
  Stats.create("events.udder", Counters.EventsUdder);
  Stats.create("events.geometry_errors", Counters.GeometryErrors);

  Stats.create("transmit.bytes", Counters.TxBytes);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", Counters.kafka_produce_fails);
  Stats.create("kafka.ev_errors", Counters.kafka_ev_errors);
  Stats.create("kafka.ev_others", Counters.kafka_ev_others);
  Stats.create("kafka.dr_errors", Counters.kafka_dr_errors);
  Stats.create("kafka.dr_others", Counters.kafka_dr_noerrors);
  // clang-format on

  std::function<void()> inputFunc = [this]() { LokiBase::inputThread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    LokiBase::processingThread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Loki Rx ringbuffers of size %d",
         EthernetBufferMaxEntries, EthernetBufferSize);
}

void LokiBase::inputThread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);

  UDPReceiver dataReceiver(local);
  dataReceiver.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  dataReceiver.printBufferSizes();
  dataReceiver.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  while (runThreads) {
    int readSize;
    unsigned int rxBufferIndex = EthernetRingbuffer.getDataIndex();

    EthernetRingbuffer.setDataLength(rxBufferIndex, 0);
    if ((readSize = dataReceiver.receive(EthernetRingbuffer.getDataBuffer(rxBufferIndex),
                                   EthernetRingbuffer.getMaxBufSize())) > 0) {
      EthernetRingbuffer.setDataLength(rxBufferIndex, readSize);
      XTRACE(INPUT, DEB, "Received an udp packet of length %d bytes", readSize);
      Counters.RxPackets++;
      Counters.RxBytes += readSize;

      if (InputFifo.push(rxBufferIndex) == false) {
        Counters.FifoPushErrors++;
      } else {
        EthernetRingbuffer.getNextBuffer();
      }
    }
  }
  XTRACE(INPUT, ALW, "Stopping input thread.");
  return;
}

/// \brief Generate an Udder test image in '3D' one image
/// at z = 0 and one at z = 3
void LokiBase::testImageUdder(EV42Serializer & FlatBuffer) {
  ESSGeometry LoKIGeometry(56, 512, 4, 1);
  XTRACE(PROCESS, ALW, "GENERATING TEST IMAGE!");
  Udder udderImage;
  udderImage.cachePixels(LoKIGeometry.nx(), LoKIGeometry.ny(), &LoKIGeometry);
  uint32_t TimeOfFlight = 0;
  while (runThreads) {
    static int EventCount = 0;
    if (EventCount == 0) {
      uint64_t EfuTime = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
      FlatBuffer.pulseTime(EfuTime);
    }

    auto pixelId = udderImage.getPixel(LoKIGeometry.nx(), LoKIGeometry.ny(), &LoKIGeometry);
    Counters.TxBytes += FlatBuffer.addEvent(TimeOfFlight, pixelId);
    Counters.EventsUdder++;
    pixelId += LoKIGeometry.ny()*LoKIGeometry.nx()*(LoKIGeometry.nz() - 1);
    Counters.TxBytes += FlatBuffer.addEvent(TimeOfFlight, pixelId);
    Counters.EventsUdder++;

    if (EFUSettings.TestImageUSleep != 0) {
      usleep(EFUSettings.TestImageUSleep);
    }

    TimeOfFlight++;

    if (Counters.TxBytes != 0) {
      EventCount = 0;
    } else {
      EventCount++;
    }
  }
  // \todo flush everything here
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}

/// \brief Normal processing thread
void LokiBase::processingThread() {
  const unsigned int NXTubes{8};
  const unsigned int NZTubes{4};
  const unsigned int NStraws{7};
  const unsigned int NYpos{512};
  Geometry geometry(NXTubes, NZTubes, NStraws, NYpos);
  Readout ESSReadout;
  DataParser LokiParser;
  HeliumTube Amp2Pos;
  ESSTime Time;

  Producer EventProducer(EFUSettings.KafkaBroker, "LOKI_detector");

  auto Produce = [&EventProducer](auto DataBuffer, auto Timestamp) {
    EventProducer.produce(DataBuffer, Timestamp);
  };

  EV42Serializer FlatBuffer(KafkaBufferSize, "loki", Produce);

  if (EFUSettings.TestImage) {
    return testImageUdder(FlatBuffer);
  }

  unsigned int DataIndex;
  TSCTimer ProduceTimer;
  while (runThreads) {
    if (InputFifo.pop(DataIndex)) { // There is data in the FIFO - do processing
      auto DataLen = EthernetRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser
      auto DataPtr = EthernetRingbuffer.getDataBuffer(DataIndex);
      auto Res = ESSReadout.validate(DataPtr, DataLen, Readout::Loki4Amp);
      Counters.ErrorBuffer = ESSReadout.Stats.ErrorBuffer;
      Counters.ErrorSize = ESSReadout.Stats.ErrorSize;
      Counters.ErrorVersion = ESSReadout.Stats.ErrorVersion;
      Counters.ErrorTypeSubType = ESSReadout.Stats.ErrorTypeSubType;
      Counters.ErrorSeqNum = ESSReadout.Stats.ErrorSeqNum;

      if (Res != Readout::OK) {
        XTRACE(DATA, DEB, "Error parsing ESS readout header");
        continue;
      }
      XTRACE(DATA, DEB, "PulseHigh %u, PulseLow %u",
        ESSReadout.Packet.HeaderPtr->PulseHigh,
        ESSReadout.Packet.HeaderPtr->PulseLow);

      // We have good header information, now parse readout data
      Res = LokiParser.parse(ESSReadout.Packet.DataPtr, ESSReadout.Packet.DataLength);
      Counters.Readouts = LokiParser.Stats.Readouts;
      Counters.Headers = LokiParser.Stats.Headers;
      Counters.ErrorHeaders = LokiParser.Stats.ErrorHeaders;
      Counters.ErrorBytes = LokiParser.Stats.ErrorBytes;


      //Fake pulse time
      uint64_t PulseTime = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
      FlatBuffer.pulseTime(PulseTime);

      bool RealPulseTime = true;
      if (RealPulseTime) {
        auto PacketHeader = ESSReadout.Packet.HeaderPtr;
        PulseTime = Time.setReference(PacketHeader->PulseHigh,PacketHeader->PulseLow);
        XTRACE(DATA, DEB, "PulseTime (%u,%u) %" PRIu64 "", PacketHeader->PulseHigh,
         PacketHeader->PulseLow, PulseTime);
        FlatBuffer.pulseTime(PulseTime);
      }


      /// Traverse readouts
      for (auto & Section : LokiParser.Result) {
        XTRACE(DATA, DEB, "Ring %u, FEN %u", Section.RingId, Section.FENId);

        for (auto & Data : Section.Data) {
          XTRACE(DATA, DEB, "Data: time (%u, %u), FPGA %u, A %u, B %u, C %u, D %u",
            Data.TimeHigh, Data.TimeLow, Data.FpgaAndTube, Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);

          Amp2Pos.calcPositions(Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);
          /// \todo include FENId in global tube calculation, eventually
          /// \todo New format will split FPGA and Tube
          // uint64_t DataTime = Data.TimeHigh * 1000000000LU;
          // DataTime += (uint64_t)(Data.TimeLow * NsPerClock);
          // XTRACE(DATA, DEB, "DataTime %" PRIu64 "", DataTime);
          auto GlobalTube = Data.FpgaAndTube;
          auto Straw = Amp2Pos.StrawId;
          auto YPos = Amp2Pos.PosId;

          auto TimeOfFlight =  Time.getTOF(Data.TimeHigh, Data.TimeLow); // TOF in ns
          auto PixelId =  LokiModuleSettings.DetectorImage3D
              ? geometry.getPixelId3D(GlobalTube, Straw, YPos)
              : geometry.getPixelId2D(GlobalTube, Straw, YPos);
          XTRACE(EVENT, DEB, "time: %" PRIu64 ", tube %u, straw %u, ypos %u, pixel: %u",
                 TimeOfFlight, GlobalTube, Straw, YPos, PixelId);

          if (PixelId == 0) {
            Counters.GeometryErrors++;
          } else {
            Counters.TxBytes += FlatBuffer.addEvent(PulseTime, PixelId);
            Counters.Events++;
          }

        }
      } // for()

    } else {
      // There is NO data in the FIFO - do stop checks and sleep a little
      Counters.RxIdle++;
      usleep(10);
    }

    if (ProduceTimer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {

      Counters.TxBytes += FlatBuffer.produce();

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count
      Counters.kafka_produce_fails = EventProducer.stats.produce_fails;
      Counters.kafka_ev_errors = EventProducer.stats.ev_errors;
      Counters.kafka_ev_others = EventProducer.stats.ev_others;
      Counters.kafka_dr_errors = EventProducer.stats.dr_errors;
      Counters.kafka_dr_noerrors = EventProducer.stats.dr_noerrors;

      ProduceTimer.now();
    }
  }
  // \todo flush everything here
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}
} // namespace
