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
#include <common/Log.h>
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

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

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
  Stats.create("readouts.error_output_queue", Counters.ErrorOutputQueue);
  Stats.create("readouts.error_seqno", Counters.ErrorSeqNum);
  // LoKI Readout Data
  Stats.create("readouts.count", Counters.Readouts);
  Stats.create("readouts.headers", Counters.Headers);
  Stats.create("readouts.error_bytes", Counters.ErrorBytes);
  Stats.create("readouts.error_header", Counters.ErrorHeaders);

  //
  Stats.create("thread.input_idle", Counters.RxIdle);
  Stats.create("thread.processing_idle", Counters.ProcessingIdle);

  Stats.create("events.count", Counters.Events);
  Stats.create("events.udder", Counters.EventsUdder);
  Stats.create("events.calib_errors", Counters.CalibrationErrors);
  Stats.create("events.mapping_errors", Counters.MappingErrors);
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
  dataReceiver.setBufferSizes(EFUSettings.TxSocketBufferSize,
                              EFUSettings.RxSocketBufferSize);
  dataReceiver.printBufferSizes();
  dataReceiver.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  while (runThreads) {
    int readSize;

    unsigned int rxBufferIndex = RxRingbuffer.getDataIndex();

    RxRingbuffer.setDataLength(rxBufferIndex, 0);

    if ((readSize = dataReceiver.receive(RxRingbuffer.getDataBuffer(rxBufferIndex),
                                   RxRingbuffer.getMaxBufSize())) > 0) {
      RxRingbuffer.setDataLength(rxBufferIndex, readSize);
      XTRACE(INPUT, DEB, "Received an udp packet of length %d bytes", readSize);
      Counters.RxPackets++;
      Counters.RxBytes += readSize;

      if (InputFifo.push(rxBufferIndex) == false) {
        Counters.FifoPushErrors++;
      } else {
        RxRingbuffer.getNextBuffer();
      }
    } else {
      Counters.RxIdle++;
    }

  }
  XTRACE(INPUT, ALW, "Stopping input thread.");
  return;
}

/// \brief Generate an Udder test image in '3D' one image
/// at z = 0 and one at z = 3
/// \todo will not work for the full detector
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

/// \brief helper function to calculate pixels from knowledge about
/// loki panel, FENId and a single readout dataset
uint32_t LokiBase::calcPixel(
    PanelGeometry & Panel, uint8_t FEN,
    DataParser::LokiReadout & Data,
    ESSGeometry * Geometry) {

  uint8_t TubeGroup = FEN - 1;
  /// \todo validate this assumption from LoKI readout data
  /// FPGAId: 2 bits
  /// TUBE: 1 bit
  /// LocalTube: 0 - 7
  uint8_t LocalTube = ((Data.FPGAId & 0x3) << 1) + (Data.TubeId & 0x1);

  Amp2Pos.calcPositions(Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);
  auto Straw = Amp2Pos.StrawId;
  auto XPos = Amp2Pos.PosId;

  auto YPos =  Panel.getGlobalStrawId(TubeGroup, LocalTube, Straw);

  uint32_t PixelId = Geometry->pixel2D(XPos,YPos);
  XTRACE(EVENT, DEB, "xpos %u, ypos %u, pixel: %u", XPos, YPos, PixelId);

  return PixelId;
}

/// \brief Normal processing thread
void LokiBase::processingThread() {
  LokiConfiguration = Config(LokiModuleSettings.ConfigFile);

  Amp2Pos.setResolution(LokiConfiguration.Resolution);

  if (LokiModuleSettings.CalibFile.empty()) {
    uint32_t MaxPixels = LokiConfiguration.getMaxPixel();
    LokiCalibration.nullCalibration(MaxPixels);
  } else {
    LokiCalibration = Calibration(LokiModuleSettings.CalibFile);
  }

  if (LokiCalibration.getMaxPixel() != LokiConfiguration.getMaxPixel()) {
      LOG(PROCESS, Sev::Error, "Error: pixel mismatch Config ({}) and Calib ({})",
        LokiConfiguration.getMaxPixel(), LokiCalibration.getMaxPixel());
      throw std::runtime_error("Pixel mismatch");
  }

  uint32_t MaxPixel = LokiCalibration.getMaxPixel();

  std::shared_ptr<ReadoutFile> DumpFile;
  if (!LokiModuleSettings.FilePrefix.empty()) {
    DumpFile = ReadoutFile::create(LokiModuleSettings.FilePrefix + "loki_" + timeString());
  }

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
      auto DataLen = RxRingbuffer.getDataLength(DataIndex);
      if (DataLen == 0) {
        Counters.FifoSeqErrors++;
        continue;
      }

      /// \todo use the Buffer<T> class here and in parser?
      /// \todo avoid copying by passing reference to stats like for gdgem?
      auto DataPtr = RxRingbuffer.getDataBuffer(DataIndex);
      auto Res = ESSReadoutParser.validate(DataPtr, DataLen, ReadoutParser::Loki4Amp);
      Counters.ErrorBuffer = ESSReadoutParser.Stats.ErrorBuffer;
      Counters.ErrorSize = ESSReadoutParser.Stats.ErrorSize;
      Counters.ErrorVersion = ESSReadoutParser.Stats.ErrorVersion;
      Counters.ErrorTypeSubType = ESSReadoutParser.Stats.ErrorTypeSubType;
      Counters.ErrorOutputQueue = ESSReadoutParser.Stats.ErrorOutputQueue;
      Counters.ErrorSeqNum = ESSReadoutParser.Stats.ErrorSeqNum;

      if (Res != ReadoutParser::OK) {
        XTRACE(DATA, DEB, "Error parsing ESS readout header");
        continue;
      }
      XTRACE(DATA, DEB, "PulseHigh %u, PulseLow %u",
        ESSReadoutParser.Packet.HeaderPtr->PulseHigh,
        ESSReadoutParser.Packet.HeaderPtr->PulseLow);

      // We have good header information, now parse readout data
      Res = LokiParser.parse(ESSReadoutParser.Packet.DataPtr, ESSReadoutParser.Packet.DataLength);

      //Fake pulse time
      uint64_t PulseTime = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
      FlatBuffer.pulseTime(PulseTime);

      bool RealPulseTime = true;
      if (RealPulseTime) {
        auto PacketHeader = ESSReadoutParser.Packet.HeaderPtr;
        PulseTime = Time.setReference(PacketHeader->PulseHigh,PacketHeader->PulseLow);
        XTRACE(DATA, DEB, "PulseTime (%u,%u) %" PRIu64 "", PacketHeader->PulseHigh,
         PacketHeader->PulseLow, PulseTime);
        FlatBuffer.pulseTime(PulseTime);
      }


      /// Traverse readouts
      /// \todo include FENId in global tube calculation, eventually
      /// \todo New format will split FPGA and Tube
      for (auto & Section : LokiParser.Result) {
        XTRACE(DATA, DEB, "Ring %u, FEN %u", Section.RingId, Section.FENId);

        if (Section.RingId >= LokiConfiguration.Panels.size()) {
          XTRACE(DATA, WAR, "RINGId %d is incompatible with configuration", Section.RingId);
          Counters.MappingErrors++;
          continue;
        }

        PanelGeometry & Panel = LokiConfiguration.Panels[Section.RingId];

        if ((Section.FENId == 0) or (Section.FENId > Panel.getMaxGroup())) {
          XTRACE(DATA, WAR, "FENId %d outside valid range 1 - %d", Section.FENId, Panel.getMaxGroup());
          Counters.MappingErrors++;
          continue;
        }

        for (auto & Data : Section.Data) {


          auto TimeOfFlight =  Time.getTOF(Data.TimeHigh, Data.TimeLow); // TOF in ns

          XTRACE(DATA, DEB, "  Data: time (%u, %u), FPGA %u, Tube %u, A %u, B %u, C %u, D %u",
            Data.TimeHigh, Data.TimeLow, Data.FPGAId, Data.TubeId, Data.AmpA, Data.AmpB, Data.AmpC, Data.AmpD);

          // uint64_t DataTime = Data.TimeHigh * 1000000000LU;
          // DataTime += (uint64_t)(Data.TimeLow * NsPerClock);
          // XTRACE(DATA, DEB, "DataTime %" PRIu64 "", DataTime);

          auto PixelId = calcPixel(Panel, Section.FENId, Data, LokiConfiguration.Geometry);
          if (PixelId > MaxPixel) {
            Counters.CalibrationErrors++;
            continue;
          }
          // <<<< End calculating pixel values here >>>>

          // <<<< Apply calibration data here >>>>
          uint32_t MappedPixelId = LokiCalibration.Mapping[PixelId];
          XTRACE(EVENT, DEB, "time: %" PRIu64 ", pixel: %u(mapped %u)",
                 TimeOfFlight, PixelId, MappedPixelId);

          if (DumpFile) {
            Readout CurrentReadout;
            CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
            CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
            CurrentReadout.EventTimeHigh = Data.TimeHigh;
            CurrentReadout.EventTimeLow = Data.TimeLow;
            CurrentReadout.AmpA = Data.AmpA;
            CurrentReadout.AmpB = Data.AmpB;
            CurrentReadout.AmpC = Data.AmpC;
            CurrentReadout.AmpD = Data.AmpD;
            CurrentReadout.RingId = Section.RingId;
            CurrentReadout.FENId = Section.FENId;
            CurrentReadout.FPGAId = Data.FPGAId;
            CurrentReadout.TubeId = Data.TubeId;
            DumpFile->push(CurrentReadout);
          }

          if (PixelId == 0) {
            Counters.GeometryErrors++;
          } else {
            Counters.TxBytes += FlatBuffer.addEvent(PulseTime, MappedPixelId);
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
  XTRACE(INPUT, ALW, "Stopping processing thread.");
  return;
}
} // namespace Loki
