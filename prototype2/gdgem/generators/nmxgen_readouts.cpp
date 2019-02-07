/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <CLI/CLI.hpp>
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>

#include <gdgem/generators/ReaderReadouts.h>
#include <libs/include/Socket.h>
#include <unistd.h>
// GCOVR_EXCL_START


struct {
  std::string FileName{""};
  std::string IpAddress{"127.0.0.1"};
  uint16_t UDPPort{9000};
  uint64_t NumberOfPackets{0}; // 0 == all packets
  uint64_t SpeedThrottle{0}; // 0 is fastest higher is slower
  unsigned int UpdateIntervalSecs{1};
  int UDPTxBufferSize{1000000};
} Settings;

CLI::App app{"Readout to UDP data generator for Gd-GEM"};

static constexpr int TscMHz {2900};

int main(int argc, char *argv[]) {

  app.add_option("-f,--file", Settings.FileName, "Multiblade H5 file with raw readouts");
  app.add_option("-i, --ip", Settings.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Settings.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Settings.NumberOfPackets, "Number of packets to send");
  app.add_option("-t, --throttle", Settings.SpeedThrottle, "Speed throttle (0 is fastest, larger is slower)");
  CLI11_PARSE(app, argc, argv);

  if (Settings.FileName.empty()) {
    printf("No input file specified, exiting.\n");
    return 1;
  }

  hdf5::error::Singleton::instance().auto_print(false);
  //  hdf5::error::auto_print(false);

  char buffer[9000];

  const int B1M = 1000000;
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Settings.IpAddress.c_str(), Settings.UDPPort);

  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(Settings.UDPTxBufferSize, 0);
  DataSource.printBufferSizes();

  Gem::ReaderReadouts file(Settings.FileName);


  uint64_t Bytes{0};
  uint64_t TotBytes{0};
  uint64_t Packets{0};
  uint64_t TotPackets{0};

  TSCTimer ReportTimer;
  Timer USClock;

  size_t SentPackets{0}; // counter to determine when to break
  for (;;) {
    int readsz = file.read(buffer);

    if (readsz > 0 && (Settings.NumberOfPackets == 0 || SentPackets < Settings.NumberOfPackets)) {
      DataSource.send(buffer, readsz);
      SentPackets++;
      Bytes += readsz;
      Packets++;
    } else {
      std::cout << "Sent " << TotBytes + Bytes << " bytes"
                << " in " << TotPackets + Packets << " packets." << std::endl;
      std::cout << "done" << std::endl;
      return 0;
    }

    if (unlikely((ReportTimer.timetsc() / TscMHz) >= Settings.UpdateIntervalSecs * 1000000)) {
      auto usecs = USClock.timeus();
      TotBytes += Bytes;
      TotPackets += Packets;
      printf("Tx rate: %8.2f Mbps (%.2f pps), tx %5" PRIu64
             " MB (total: %7" PRIu64 " MB) %" PRIu64 " usecs\n",
             Bytes * 8.0 / usecs, Packets * 1000000.0 / usecs, Bytes / B1M,
             TotBytes / B1M, usecs);
      Bytes = 0;
      Packets = 0;
      USClock.now();
      ReportTimer.now();
    }
    usleep(Settings.SpeedThrottle * 1000);
  }

  return 0;
}
// GCOVR_EXCL_STOP
