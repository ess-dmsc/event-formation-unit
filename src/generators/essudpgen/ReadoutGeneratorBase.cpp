// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator base class of artificial ESS readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <CLI/Error.hpp>
#include <common/debug/Trace.h>
#include <common/time/ESSTime.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctype.h>
#include <memory>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

using namespace ESSReadout;

///\brief Constructor initialize the generator app
ReadoutGeneratorBase::ReadoutGeneratorBase(DetectorType Type) {
  // Store the detector type
  Settings.Detector = Type;

  // clang-format off
  //
  // Options
  app.add_option("-i, --ip",             Settings.IpAddress,      "Destination IP address");
  app.add_option("-p, --port",           Settings.UDPPort,        "Destination UDP port");
  app.add_option("-a, --packets",        Settings.NumberOfPackets,"Number of packets to send");
  app.add_option("-t, --throttle",       Settings.SpeedThrottle,  "Speed throttle (0 is fastest, larger is slower)");
  app.add_option("-s, --pkt_throttle",   Settings.PktThrottle,    "Number of microseconds to pause after n packets");
  app.add_option("-y, --type",           Settings.Detector,       "Detector type id");
  app.add_option("-f, --fibers",         Settings.NFibers,        "Number of fibers used in data header");
  app.add_option("-q, --frequency",      Settings.Frequency,      "Pulse frequency in Hz. (default 0: refreshed for each packet)");
  app.add_option("-v, --header_version", Settings.HeaderVersion,  "Header version, v1 by default");
  app.add_option("--time_source",        Settings.TimeSource,     "Bit mask defining activated bits of the time source field");

  // Flags
  app.add_flag("-r, --random",  Settings.Randomise, "Randomise header and data fields");
  app.add_flag("-l, --loop",    Settings.Loop,      "Run forever");
  app.add_flag("--debug",       Settings.Debug,     "print debug information");
  //
  // clang-format on


  pulseTime = ESSTime::now();
  prevPulseTime = pulseTime;
}

std::pair<uint32_t, uint32_t>
ReadoutGeneratorBase::generateReadoutTime() const {
  if (readoutTimeGenerator == nullptr) {
    throw std::runtime_error("Readout time generator is not initialized");
  }

  TimeDurationMilli t = static_cast<TimeDurationMilli>(readoutTimeGenerator->getValue());
  ESSTime readoutTime = pulseTime + esstime::msToNanoseconds(t);

  return {readoutTime.getTimeHigh(), readoutTime.getTimeLow()};
}

std::pair<uint32_t, uint32_t>
ReadoutGeneratorBase::generateReadoutTimeEveryN(int EveryN) {
  static int Counter = 0;

  if ((Counter % EveryN) == 0) {
    std::tie(cachedTimeHigh, cachedTimeLow) = generateReadoutTime();
  }
  Counter++;
  return {cachedTimeHigh, cachedTimeLow};
}

TimeDurationNano
ReadoutGeneratorBase::getTimeOfFlightNS(ESSTime &readoutTime) const {
  return readoutTime - pulseTime;
}

void ReadoutGeneratorBase::generatePackets(
    SocketInterface *socket, const TimeDurationNano &pulseTimeDuration) {
  assert(ReadoutDataSize != 0); // must be set in generator application
  pulseTime = ESSTime::now();
  prevPulseTime = pulseTime - pulseTimeDuration;
  const TimeDurationNano start = pulseTime.toNS();
  do {
    generateHeader();
    generateData();
    finishPacket();
    socket->send(&Buffer[0], DataSize);
    Packets++;
    if (Settings.PktThrottle) {
      if (Packets % Settings.PktThrottle == 0) {
        usleep(10);
      }
    }
    if (Settings.NumberOfPackets != 0 and Packets >= Settings.NumberOfPackets) {
      break;
    }
    if (Settings.SpeedThrottle) {
      usleep(Settings.SpeedThrottle);
    }
  } while (std::chrono::high_resolution_clock::now().time_since_epoch() -
               start <
           pulseTimeDuration);
  numberOfReadouts = 0; // reset readout counter for next packet
}

void ReadoutGeneratorBase::setReadoutDataSize(uint8_t ReadoutSize) {
  ReadoutDataSize = ReadoutSize;
}

void ReadoutGeneratorBase::setReadoutPerPacket(uint32_t ReadoutCount) {
  ReadoutsPerPacket = ReadoutCount;
}

void ReadoutGeneratorBase::generateHeader() {

  DataSize = HeaderSize + ReadoutsPerPacket * ReadoutDataSize;
  if (DataSize > BufferSize) {
    throw std::runtime_error("Too many readouts for buffer size");
  }

  memset(Buffer, 0, BufferSize);
  auto Header = reinterpret_cast<Parser::PacketHeaderV0 *>(Buffer);

  Header->CookieAndType = (Settings.Detector << 24) + 0x535345;
  Header->Padding0 = 0;

  if (headerVersion == Parser::HeaderVersion::V0) {
    Header->Version = 0;
  } else {
    Header->Version = 1;
  }

  // Header->OutputQueue = 0x00;
  Header->TotalLength = DataSize;
  Header->SeqNum = SeqNum;

  Header->PulseHigh = pulseTime.getTimeHigh();
  Header->PulseLow = pulseTime.getTimeLow();
  Header->PrevPulseHigh = prevPulseTime.getTimeHigh();
  Header->PrevPulseLow = prevPulseTime.getTimeLow();
  Header->TimeSource = Settings.TimeSource;

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

  // Estimate how many packages it is possible to generate per pulse.
  TimeDurationNano pulseTimeDuration{
      static_cast<int64_t>(1000000000 / Settings.Frequency)};

  XTRACE(DATA, INF,
         "First pulse time generated, High: %u, Low: %u, periodNs: %u",
         pulseTime.getTimeHigh(), pulseTime.getTimeLow(), pulseFrequencyNs);

  do {
    generatePackets(DataSource.get(), pulseTimeDuration);

    // printf("Sent %" PRIu64 " packets\n", TotalPackets);
  } while (Settings.Loop or Packets < Settings.NumberOfPackets);
  // pcap.printstats();
  printf("Sent %" PRIu64 " packets\n", Packets);
}

void ReadoutGeneratorBase::initialize(
    std::unique_ptr<FunctionGenerator> readoutGenerator) {
  SocketImpl::Endpoint local("0.0.0.0", 0);
  SocketImpl::Endpoint remote(Settings.IpAddress.c_str(), Settings.UDPPort);

  DataSource = std::make_unique<UDPTransmitter>(local, remote);
  DataSource->setBufferSizes(Settings.KernelTxBufferSize, 0);
  DataSource->printBufferSizes();

  // Parse the header version
  switch (Settings.HeaderVersion) {
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

  // Figure out distribution that will be used for the readoutGenerator.
  readoutTimeGenerator = std::move(readoutGenerator);

  pulseFrequencyNs = esstime::hzToNanoseconds(Settings.Frequency);
  if (ReadoutsPerPacket == 0) {
    ReadoutsPerPacket = (BufferSize - HeaderSize) / ReadoutDataSize;
    // Ensure we have an even number of readouts
    if (ReadoutsPerPacket % 2) {
      ReadoutsPerPacket -= 1;
    }
  }
  XTRACE(DATA, INF, "Frequency defined as %u ns", pulseFrequencyNs);
}

// GCOVR_EXCL_STOP
