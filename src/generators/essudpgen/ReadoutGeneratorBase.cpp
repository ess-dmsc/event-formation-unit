// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator base class of artificial ESS readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include "common/time/ESSTime.h"
#include <Error.hpp>
#include <common/debug/Trace.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctype.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

using namespace ESSReadout;

///\brief Constructor initialize the generator app
ReadoutGeneratorBase::ReadoutGeneratorBase(DetectorType Type) {
  // Store the detector type
  Settings.Detector = Type;

  // Options
  app.add_option("-i, --ip", Settings.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Settings.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Settings.NumberOfPackets,
                 "Number of packets to send");
  app.add_option("-t, --throttle", Settings.SpeedThrottle,
                 "Speed throttle (0 is fastest, larger is slower)");
  app.add_option("-s, --pkt_throttle", Settings.PktThrottle,
                 "Extra usleep() after n packets");
  app.add_option("-y, --type", Settings.Detector, "Detector type id");
  app.add_option("-f, --fibers", Settings.NFibers,
                 "Number of Fibers used in data header");
  app.add_option("-q, --frequency", Settings.Frequency,
                 "Pulse frequency in Hz. (default 0: refreshed for "
                 "each packet)");
  app.add_option("-v, --header_version", Settings.headerVersion,
                 "Header version, v1 by default");

  // Flags
  app.add_flag("-r, --random", Settings.Randomise,
               "Randomise header and data fields");
  app.add_flag("-l, --loop", Settings.Loop, "Run forever");
  app.add_flag("--debug", Settings.Debug, "print debug information");

  // Set pulse time and readout time. Previous pulse time will be set to wrong
  // value and must be set after this.
  UpdateTimestamps(true);
  // Setting previous pulse time to current pulse time. At this point there have
  // not been any pulse
  prevPulseTime = pulseTime;
}

std::pair<uint32_t, uint32_t> ReadoutGeneratorBase::getReadOutTimes() {
  return getReadOutTimes(distributionGenerator->getValue());
}

std::pair<uint32_t, uint32_t>
ReadoutGeneratorBase::getReadOutTimes(double timeOfFlightMs) {
  ESSTime readoutTime = pulseTime + esstime::msToNanosecounds(timeOfFlightMs);
  return {
    readoutTime.getTimeHigh(),
    readoutTime.getTimeLow()
  };
}

void ReadoutGeneratorBase::UpdateTimestamps(bool updateTime) {
  if (updateTime) {
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    prevPulseTime = pulseTime;
    pulseTime =
        ESSTime(std::chrono::duration_cast<TimeDurationNano>(now));
  }

  readoutTime = pulseTime;
}

void ReadoutGeneratorBase::generatePackets(
    SocketInterface *socket,
    const TimeDurationNano &pulseTimeDuration) {
  assert(ReadoutDataSize != 0); // must be set in generator application
  UpdateTimestamps(true);
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
    UpdateTimestamps(false);
  } while (std::chrono::high_resolution_clock::now().time_since_epoch() -
               start <
           pulseTimeDuration);
}

void ReadoutGeneratorBase::setReadoutDataSize(uint8_t ReadoutSize) {
  ReadoutDataSize = ReadoutSize;
}

void ReadoutGeneratorBase::setNumberOfReadouts(uint32_t ReadoutCount) {
  NumberOfReadouts = ReadoutCount;
}

void ReadoutGeneratorBase::generateHeader() {

  DataSize = HeaderSize + NumberOfReadouts * ReadoutDataSize;
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
    generatePackets(DataSource, pulseTimeDuration);

    // printf("Sent %" PRIu64 " packets\n", TotalPackets);
  } while (Settings.Loop or Packets < Settings.NumberOfPackets);
  // pcap.printstats();
  printf("Sent %" PRIu64 " packets\n", Packets);
}

void ReadoutGeneratorBase::initialize(
    std::shared_ptr<FunctionGenerator> generator) {
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

  // Figure out distribution that will be used for the generator.
  distributionGenerator = generator;
  pulseFrequencyNs = esstime::hzToNanoseconds(Settings.Frequency);
  if (NumberOfReadouts == 0)
    NumberOfReadouts = (BufferSize - HeaderSize) / ReadoutDataSize;
  XTRACE(DATA, INF, "Frequency defined as %u ns", pulseFrequencyNs);
}

// GCOVR_EXCL_STOP
