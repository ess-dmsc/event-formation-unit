// Copyright (C) 2021 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator base class of artificial ESS readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <common/debug/Trace.h>
#include <cstdio>
#include <generators/essudpgen/ReadoutGeneratorBase.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

using namespace ESSReadout;

///\brief Constructor initialize the generator app
ReadoutGeneratorBase::ReadoutGeneratorBase() {
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
  app.add_option("--p1", Settings.FreeParam1,
                "Free parameter for custom purposes");
  app.add_option("--p2", Settings.FreeParam2,
                "Free parameter for custom purposes");
  app.add_flag("-m, --random", Settings.Randomise,
               "Randomise header and data fields");
  app.add_flag("-l, --loop", Settings.Loop, "Run forever");
}

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

  // time current time for pulse time high
  PulseTimeHigh = time(NULL);

  Header->PulseHigh = PulseTimeHigh;
  Header->PulseLow = TimeLowOffset;
  Header->PrevPulseHigh = PulseTimeHigh;
  Header->PrevPulseLow = PrevTimeLowOffset;

  if (headerVersion == Parser::HeaderVersion::V1) {
    auto HeaderV1 = reinterpret_cast<Parser::PacketHeaderV1 *>(Buffer);
    HeaderV1->CMACPadd = 0;
  }

  XTRACE(DATA, DEB, "new packet header, time high %u, time low %u", PulseTimeHigh,
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
  CLI11_PARSE(app, argc, argv);

  // if help was requested, return -1
  if (app.get_help_ptr()) {
    return -1;
  }

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
}

// GCOVR_EXCL_STOP
