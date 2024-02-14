// Copyright (C) 2021 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial ESS readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <CLI/CLI.hpp>
#include <common/debug/Trace.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

using namespace ESSReadout;
///\brief No work to do for constructor
ReadoutGeneratorBase::ReadoutGeneratorBase() {}

///\brief
uint16_t ReadoutGeneratorBase::makePacket() {
  assert(ReadoutDataSize != 0); // must be set in generator application
  generateHeader();
  generateData();
  finishPacket();
  return DataSize;
}

void ReadoutGeneratorBase::generateHeader() {
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

  DataSize = HeaderSize + Settings.NumReadouts * ReadoutDataSize;
  if (DataSize >= BufferSize) {
    throw std::runtime_error("Too many readouts for buffer size");
  }

  memset(Buffer, 0, BufferSize);
  auto Header = reinterpret_cast<Parser::PacketHeaderV1 *>(Buffer);

  if (headerVersion == Parser::HeaderVersion::V1) {
    Header = reinterpret_cast<Parser::PacketHeaderV1 *>(Buffer);
  }

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

  TimeHigh = time(NULL);

  Header->PulseHigh = TimeHigh;
  Header->PulseLow = TimeLowOffset;
  Header->PrevPulseHigh = TimeHigh;
  Header->PrevPulseLow = PrevTimeLowOffset;
  
  if (headerVersion == Parser::HeaderVersion::V1) {
    Header->CMACPadd = 0;
  }

  XTRACE(DATA, DEB, "new packet header, time high %u, time low %u", TimeHigh,
         TimeLowOffset);
}

void ReadoutGeneratorBase::finishPacket() {
  SeqNum++; // ready for next packet

  // if doing fuzzing, fuzz up to one field in header & up to 20 fields in data
  if (Settings.Randomise) {
    Fuzzer.fuzz8Bits(Buffer, HeaderSize, 1);
    Fuzzer.fuzz8Bits(Buffer + HeaderSize, DataSize - HeaderSize, 20);
  }
}

int ReadoutGeneratorBase::argParse(int argc, char *argv[]) {
  CLI::App app{"UDP data generator for ESS readout data"};

  app.add_option("-i, --ip", Settings.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Settings.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Settings.NumberOfPackets,
                 "Number of packets to send");
  app.add_option("-t, --throttle", Settings.SpeedThrottle,
                 "Speed throttle (0 is fastest, larger is slower)");
  app.add_option("-s, --pkt_throttle", Settings.PktThrottle,
                 "Extra usleep() after n packets");
  app.add_option("-y, --type", Settings.TypeOverride, "Detector type id");
  app.add_option("-r, --rings", Settings.NFibers,
                 "Number of Fibers used in data header (obsolete)");
  app.add_option("-f, --fibers", Settings.NFibers,
                 "Number of Fibers used in data header");
  app.add_option("-e, --ev_delay", Settings.TicksBtwEvents,
                 "Delay (ticks) between events");
  app.add_option("-d, --rd_delay", Settings.TicksBtwReadouts,
                 "Delay (ticks) between coincident readouts");
  app.add_option("-o, --readouts", Settings.NumReadouts,
                 "Number of readouts per packet");
  app.add_option("-v, --header_version", Settings.headerVersion,
                 "Header version, v1 by default");
  app.add_flag("-m, --random", Settings.Randomise,
               "Randomise header and data fields");
  app.add_flag("-l, --loop", Settings.Loop, "Run forever");

  CLI11_PARSE(app, argc, argv);
  return 0;
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
      printf("Sent %" PRIu64 " packets\n", Packets);
      break;
    }
    // printf("Sent %" PRIu64 " packets\n", TotalPackets);
  } while (Settings.Loop or Packets < Settings.NumberOfPackets);
  // pcap.printstats();
}

void ReadoutGeneratorBase::main() {
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Settings.IpAddress.c_str(), Settings.UDPPort);

  DataSource = new UDPTransmitter(local, remote);
  DataSource->setBufferSizes(Settings.KernelTxBufferSize, 0);
  DataSource->printBufferSizes();
}

// GCOVR_EXCL_STOP
