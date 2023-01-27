// Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial ESS readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <CLI/CLI.hpp>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <cassert>
#include <math.h>
#include <stdexcept>
#include <time.h>
#include <common/debug/Trace.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB


///\brief No work to do for constructor
ReadoutGeneratorBase::ReadoutGeneratorBase() { }

///\brief
uint16_t ReadoutGeneratorBase::makePacket() {
  assert(ReadoutDataSize != 0); // must be set in generator application
  generateHeader();
  generateData();
  finishPacket();
  return DataSize;
}

void ReadoutGeneratorBase::generateHeader() {
  assert(HeaderSize == 30);
  DataSize = HeaderSize + Settings.NumReadouts * ReadoutDataSize;
  if (DataSize >= BufferSize) {
    throw std::runtime_error("Too many readouts for buffer size");
  }

  memset(Buffer, 0, BufferSize);
  auto Header = (ESSReadout::Parser::PacketHeaderV0 *)Buffer;

  Header->CookieAndType = (Settings.Type << 24) + 0x535345;
  Header->Padding0 = 0;
  Header->Version = 0;
  // Header->OutputQueue = 0x00;

  Header->TotalLength = DataSize;
  Header->SeqNum = SeqNum;

  TimeHigh = time(NULL);

  Header->PulseHigh = TimeHigh;
  Header->PulseLow = TimeLowOffset;
  Header->PrevPulseHigh = TimeHigh;
  Header->PrevPulseLow = PrevTimeLowOffset;

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


int ReadoutGeneratorBase::argParse(int argc, char * argv[]) {
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
  app.add_option("-r, --rings", Settings.NRings,
                 "Number of Rings used in data header");
  app.add_option("-e, --ev_delay", Settings.TicksBtwEvents,
                 "Delay (ticks) between events");
  app.add_option("-d, --rd_delay", Settings.TicksBtwReadouts,
                 "Delay (ticks) between coincident readouts");
  app.add_option("-o, --readouts", Settings.NumReadouts,
                 "Number of readouts per packet");
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
