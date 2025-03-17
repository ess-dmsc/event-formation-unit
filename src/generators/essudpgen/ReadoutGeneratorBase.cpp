// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator base class of artificial ESS readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <Error.hpp>
#include <common/debug/Trace.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

#include <chrono>
#include <cstdlib>
#include <ctype.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

using namespace ESSReadout;

///\brief Constructor initialize the generator app
ReadoutGeneratorBase::ReadoutGeneratorBase(Parser::DetectorType Type) {
  // Store the detector type
  Settings.Type = Type;

  // Options
  app.add_option("-i, --ip", Settings.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Settings.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Settings.NumberOfPackets,
                 "Number of packets to send");
  app.add_option("-t, --throttle", Settings.SpeedThrottle,
                 "Speed throttle (0 is fastest, larger is slower)");
  app.add_option("-s, --pkt_throttle", Settings.PktThrottle,
                 "Extra usleep() after n packets");
  app.add_option("-y, --type", Settings.TypeOverride, "Detector type id");
  
  app.add_option("-f, --fibers", Settings.NFibers,   "Number of Fibers used in data header");
  app.add_option("--fibermask",  Settings.FiberMask, "Mask out unused fibers");


  app.add_option("-q, --frequency", Settings.Frequency,
                 "Pulse frequency in Hz. (default 0: refreshed for "
                 "each packet)");
  app.add_option("-e, --ev_delay", Settings.TicksBtwEvents,
                 "Delay (ticks) between events");
  app.add_option("-d, --rd_delay", Settings.TicksBtwReadouts,
                 "Delay (ticks) between coincident readouts");
  app.add_option("-o, --readouts", Settings.NumReadouts,
                 "Number of readouts per packet");
  app.add_option("-v, --header_version", Settings.headerVersion,
                 "Header version, v1 by default");

  // Flags
  app.add_flag("-r, --random", Settings.Randomise,
               "Randomise header and data fields");
  app.add_flag("-l, --loop", Settings.Loop, "Run forever");

  // Look-up convenience
  NameToType["CBM"]      = Parser::CBM;
  NameToType["LOKI"]     = Parser::LOKI;
  NameToType["TBL3HE"]   = Parser::TBL3HE;
  NameToType["BIFROST"]  = Parser::BIFROST;
  NameToType["MIRACLES"] = Parser::MIRACLES;
  NameToType["CSPEC"]    = Parser::CSPEC;
  NameToType["TREX"]     = Parser::TREX;
  NameToType["NMX"]      = Parser::NMX;
  NameToType["FREIA"]    = Parser::FREIA;
  NameToType["TBLVMM"]   = Parser::TBLVMM;
  NameToType["ESTIA"]    = Parser::ESTIA;
  NameToType["DREAM"]    = Parser::DREAM;
  NameToType["MAGIC"]    = Parser::MAGIC;
  NameToType["HEIMDAL"]  = Parser::HEIMDAL;
}

void ReadoutGeneratorBase::setDetectorType(const std::string &Name) {
  std::string str = Name;
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
  setDetectorType(NameToType[str]);
}

void ReadoutGeneratorBase::setDetectorType(Parser::DetectorType Type) {
  Settings.Type = Type;
}

uint16_t ReadoutGeneratorBase::makePacket() {
  assert(ReadoutDataSize != 0); // must be set in generator application
  generateHeader();
  generateData();
  finishPacket();
  return DataSize;
}

void ReadoutGeneratorBase::setReadoutDataSize(uint8_t ReadoutSize) {
  ReadoutDataSize = ReadoutSize;
}

void ReadoutGeneratorBase::generateHeader() {

  DataSize = HeaderSize + NumberOfReadouts * ReadoutDataSize;
  if (DataSize > BufferSize) {
    throw std::runtime_error("Too many readouts for buffer size");
  }

  memset(Buffer, 0, BufferSize);
  auto Header = reinterpret_cast<Parser::PacketHeaderV0 *>(Buffer);

  Header->CookieAndType = (Settings.Type << 24) + 0x535345;
  Header->Padding0 = 0;

  if (headerVersion == Parser::HeaderVersion::V0) {
    Header->Version = 0;
  } else {
    Header->Version = 1;
  }

  // Header->OutputQueue = 0x00;
  Header->TotalLength = DataSize;
  Header->SeqNum = SeqNum;

  // Generate pulse time first time
  if (pulseTime.getTimeHigh() == 0 && pulseTime.getTimeLow() == 0) {
    pulseTime = ESSTime(time(NULL), 0);
    readoutTime = pulseTime;
    prevPulseTime = pulseTime;

    XTRACE(DATA, INF,
           "First pulse time generated, High: %u, Low: %u, periodNs: %u",
           pulseTime.getTimeHigh(), pulseTime.getTimeLow(), pulseFrequencyNs);
  }

  // If pulseFrequencyNs is set, we generate pulse time
  // by adding the pulse time window according to the pulse frequency
  // which will simulate the real operation of the data source
  if (pulseFrequencyNs != TimeDurationNano(0)) {

    // if the readout time is greater than the pulse time, generate new
    // pulse time for the header
    if (readoutTime.toNS() >= getNextPulseTimeNs()) {

      // calculate the absolute time difference between the real time and the
      // pulse time in milliseconds, because we do not need high precision
      auto RealAndPulseTimeDiff =
          abs(sToMilliseconds(time(NULL)) -
              nsToMicroseconds(pulseTime.toNS().count()));

      /// \note: reset the pulse time if it has drifted too much from
      /// real clock. This is a workaround for the file writer not accepting the
      /// pulse time if it is too far from the real clock
      if (RealAndPulseTimeDiff > sToMilliseconds(MAX_TIME_DRIFT)) {

        XTRACE(DATA, WAR, "Pulse time has drifted too much, reset it");
        pulseTime = ESSTime(time(NULL), 0);
        prevPulseTime = pulseTime;
        // reset the readout time as well since we resynced our time bases
        readoutTime = pulseTime;
      } else {
        prevPulseTime = pulseTime;
        pulseTime += pulseFrequencyNs;
      }
      XTRACE(DATA, INF,
             "New pulseTime generated, High: %u, Low: %u, periodNs: %u",
             pulseTime.getTimeHigh(), pulseTime.getTimeLow(),
             pulseFrequencyNs.count());
    }
    // if the requested frequency is 0, generate a new pulse time for each
    // packet and we use fake offset btw. pulse and prevPulse

    /// \todo: This operation mode should be made obsolete as soon as
    /// the infrastructure is updated to use frequency mode
  } else {
    prevPulseTime = ESSTime(time(NULL), PrevTimeLowOffset);
    pulseTime = ESSTime(time(NULL), TimeLowOffset);
    readoutTime = pulseTime + Settings.TicksBtwReadouts;
    XTRACE(DATA, INF,
           "New pulseTime generated for this packet, High: %u, Low: %u",
           pulseTime.getTimeHigh(), pulseTime.getTimeLow());
  }

  Header->PulseHigh = pulseTime.getTimeHigh();
  Header->PulseLow = pulseTime.getTimeLow();
  Header->PrevPulseHigh = prevPulseTime.getTimeHigh();
  Header->PrevPulseLow = prevPulseTime.getTimeLow();

  if (headerVersion == Parser::HeaderVersion::V1) {
    auto HeaderV1 = reinterpret_cast<Parser::PacketHeaderV1 *>(Buffer);
    HeaderV1->CMACPadd = 0;
  }

  XTRACE(DATA, DEB, "new packet header, time high %u, time low %u",
         Header->PulseHigh, Header->PulseLow);
}

void ReadoutGeneratorBase::finishPacket() {
  SeqNum++; // ready for next packet

  // if doing fuzzing, fuzz up to one field in header & up to 20 fields in data
  if (Settings.Randomise) {
    Fuzzer.fuzz8Bits(Buffer, HeaderSize, 1);
    Fuzzer.fuzz8Bits(Buffer + HeaderSize, DataSize - HeaderSize, 20);
  }
}

void ReadoutGeneratorBase::argParse(int argc, char *argv[]) {
  try {
    app.parse(argc, argv);
  } catch (const CLI::CallForHelp &e) {
    std::exit(app.exit(e));
  } catch (const CLI::Error &e) {
    std::exit(app.exit(e));
  }
}

void ReadoutGeneratorBase::transmitLoop() {
  if (Settings.TypeOverride != 0) {
    Settings.Type = Settings.TypeOverride;
  }

  do {
    uint16_t DataSize = makePacket();

    DataSource->send(&Buffer[0], DataSize);

    if (Settings.SpeedThrottle) {
      usleep(Settings.SpeedThrottle);
    }
    Packets++;
    if (Settings.PktThrottle) {
      if (Packets % Settings.PktThrottle == 0) {
        usleep(10);
      }
    }
    if (Settings.NumberOfPackets != 0 and Packets >= Settings.NumberOfPackets) {
      break;
    }
    // printf("Sent %" PRIu64 " packets\n", TotalPackets);
  } while (Settings.Loop or Packets < Settings.NumberOfPackets);
  // pcap.printstats();
  printf("Sent %" PRIu64 " packets\n", Packets);
}

void ReadoutGeneratorBase::main() {
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Settings.IpAddress.c_str(), Settings.UDPPort);

  DataSource = new UDPTransmitter(local, remote);
  DataSource->setBufferSizes(Settings.KernelTxBufferSize, 0);
  DataSource->printBufferSizes();

  // Parse the header version
  switch (Settings.headerVersion) {
  case Parser::HeaderVersion::V0:
    headerVersion = Parser::HeaderVersion::V0;
    HeaderSize = sizeof(Parser::PacketHeaderV0);
    break;
  case Parser::HeaderVersion::V1:
    headerVersion = Parser::HeaderVersion::V1;
    HeaderSize = sizeof(Parser::PacketHeaderV1);
    break;
  default:
    throw std::runtime_error("Incorrect header version");
  }

  if (headerVersion == Parser::HeaderVersion::V0) {
    assert(HeaderSize == 30);
  } else {
    assert(HeaderSize == 32);
  }

  if (Settings.Frequency != 0) {
    pulseFrequencyNs = esstime::hzToNanoseconds(Settings.Frequency);
    NumberOfReadouts = (BufferSize - HeaderSize) / ReadoutDataSize;
    XTRACE(DATA, INF, "Frequency defined as %u ns", pulseFrequencyNs);
  } else {
    NumberOfReadouts = Settings.NumReadouts;
  }
}

// GCOVR_EXCL_STOP
