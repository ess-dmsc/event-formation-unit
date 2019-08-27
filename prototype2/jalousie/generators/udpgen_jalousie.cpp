/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <CLI/CLI.hpp>
#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <jalousie/generators/ReaderJalousie.h>
#include <common/Socket.h>
#include <string.h>
#include <string>
#include <unistd.h>
// GCOVR_EXCL_START

struct {
  std::string FileName{""};
  std::string IpAddress{"127.0.0.1"};
  uint16_t UDPPort{9000};
  uint64_t NumberOfPackets{0}; // 0 == all packets
  uint64_t SpeedThrottle{0}; // 0 is fastest higher is slower
  uint64_t PktThrottle{0}; // 0 is fastest
  bool Loop{false}; // Keep looping the same file forever
  // Not yet CLI settings
  uint32_t KernelTxBufferSize{1000000};
} Settings;

CLI::App app{"Jalousie file to UDP data generator"};

int main(int argc, char *argv[]) {
  app.add_option("-f, --file", Settings.FileName, "Jalousie H5 file with raw readouts");
  app.add_option("-i, --ip", Settings.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Settings.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Settings.NumberOfPackets, "Number of packets to send");
  app.add_option("-t, --throttle", Settings.SpeedThrottle, "Speed throttle (0 is fastest, larger is slower)");
  app.add_option("-s, --pkt_throttle", Settings.PktThrottle, "Extra usleep() after n packets");
  app.add_flag("-l, --loop", Settings.Loop, "Run forever");
  CLI11_PARSE(app, argc, argv);

  char buffer[10000];
  const int MaxBufferSize = 8900;

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Settings.IpAddress.c_str(), Settings.UDPPort);

  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(Settings.KernelTxBufferSize, 0);
  DataSource.printBufferSizes();

  std::string JalousieFile(Settings.FileName);

  uint64_t packets = 0;
  uint64_t totpackets = 0;
  do {
    int rdsize;
    ReaderJalousie jalousie(JalousieFile);
    while ((rdsize = jalousie.read((char *)&buffer, MaxBufferSize)) != -1) {
      if (rdsize == 0) {
        printf("no data returned - ignoring\n");
        continue; //
      }

      DataSource.send(buffer, rdsize);
      if (Settings.SpeedThrottle) {
        usleep(Settings.SpeedThrottle);
      }
      packets++;
      totpackets++;
      if (Settings.PktThrottle) {
        if (totpackets % Settings.PktThrottle == 0) {
          usleep(10);
        }
      }
      if (Settings.NumberOfPackets != 0 and packets >= Settings.NumberOfPackets) {
        printf("Sent %" PRIu64 " packets\n", totpackets);
        packets = 0;
        break;
      }

    }

    printf("Sent %" PRIu64 " packets\n", totpackets);

  } while (Settings.Loop);

  return 0;
}
// GCOVR_EXCL_STOP
