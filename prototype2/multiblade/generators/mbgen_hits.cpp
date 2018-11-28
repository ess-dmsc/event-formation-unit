/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <CLI/CLI.hpp>
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <libs/include/Socket.h>
#include <multiblade/generators/ReaderHits.h>
#include <unistd.h>

// GCOVR_EXCL_START

struct {
  std::string FileName{""};
  std::string IpAddress{"127.0.0.1"};
  uint16_t UDPPort{9000};
  uint64_t NumberOfPackets{0}; // 0 == all packets
  uint64_t SpeedThrottle{0}; // 0 is fastes higher is slower
} Settings;

CLI::App app{"Readout to UDP data generator for Multi-Blade"};

const int TSC_MHZ = 2900;

int main(int argc, char *argv[]) {

  app.add_option("-f,--file", Settings.FileName, "Multiblade H5 file with raw readouts");
  app.add_option("-i, --ip", Settings.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Settings.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Settings.NumberOfPackets, "Number of packets to send");
  app.add_option("-t, --throttle", Settings.SpeedThrottle, "Speed throttle (0 is fastest, larger is slower)");

  CLI11_PARSE(app, argc, argv);

  if (Settings.FileName.empty())
    return 1;

  hdf5::error::Singleton::instance().auto_print(false);

  char buffer[9000];

  const int B1M = 1000000;
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Settings.IpAddress.c_str(), Settings.UDPPort);

  UDPTransmitter DataSource(local, remote);

  ReaderHits file(Settings.FileName);

  int readsz;

  uint64_t tx_total = 0;
  uint64_t txp_total = 0;
  uint64_t tx = 0;
  uint64_t txp = 0;

  TSCTimer report_timer;
  Timer us_clock;

  for (;;) {
    if (Settings.NumberOfPackets != 0 && txp_total >= Settings.NumberOfPackets) {
      printf("Max packet reached, exiting...\n");
      break;
    }
    //readsz = file.read(buffer);
    readsz = 1; // debug
    if (readsz > 0) {
      printf("Sending packet of size %d\n", readsz);
      DataSource.send(buffer, readsz);
      tx += readsz;
      txp++;
      tx_total += readsz;
      txp_total++;
    } else {
      std::cout << "Sent " << tx_total + tx << " bytes"
                << " in " << txp_total + txp << " packets." << std::endl;
      std::cout << "done" << std::endl;
      exit(0);
    }

    if (unlikely((report_timer.timetsc() / TSC_MHZ) >= 1 * 1000000)) {
      auto usecs = us_clock.timeus();

      printf("Tx rate: %8.2f Mbps (%.2f pps), tx %5" PRIu64
             " MB (total: %7" PRIu64 " MB) %" PRIu64 " usecs\n",
             tx * 8.0 / usecs, txp * 1000000.0 / usecs, tx / B1M,
             tx_total / B1M, usecs);
      tx = 0;
      txp = 0;
      us_clock.now();
      report_timer.now();
    }
    usleep(Settings.SpeedThrottle * 1000);
  }
  return 0;
}
// GCOVR_EXCL_STOP
