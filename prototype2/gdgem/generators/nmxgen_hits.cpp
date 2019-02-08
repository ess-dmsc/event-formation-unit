/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <CLI/CLI.hpp>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>

#include <gdgem/generators/ReaderHits.h>
#include <libs/include/Socket.h>
// GCOVR_EXCL_START

static constexpr int TscMHz {2900};

// Reader expects filenames without .h5 extension
std::string remove_extension(const std::string& filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot);
}

struct {
  std::string FileName{""};
  std::string IpAddress{"127.0.0.1"};
  uint16_t UDPPort{9000};
  uint64_t NumberOfPackets{0}; // 0 == all packets
  uint64_t SpeedThrottle{0}; // 0 is fastest higher is slower
  uint16_t MaxPacketSize{0};
  uint32_t KernelTxBufferSize{1000000};
  uint32_t UpdateIntervalSecs{1};
} Settings;

CLI::App app{"Readout to UDP data generator for Gd-GEM"};

int main(int argc, char *argv[]) {
  app.add_option("-f,--file", Settings.FileName, "Multiblade H5 file with raw readouts");
  app.add_option("-i, --ip", Settings.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Settings.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Settings.NumberOfPackets, "Number of packets to send");
  app.add_option("-b, --bytes", Settings.MaxPacketSize, "Maximum number of bytes per packet");
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
  DataSource.setBufferSizes(Settings.KernelTxBufferSize, 0);
  DataSource.printBufferSizes();

  Settings.FileName = remove_extension(Settings.FileName);
  Gem::ReaderHits file(Settings.FileName);

  size_t ReadoutSize = file.getReadoutSize();
  uint16_t MaxTxSize = ReadoutSize * file.getChunkSize();
  if (Settings.MaxPacketSize != 0) {
    MaxTxSize = (Settings.MaxPacketSize/ReadoutSize) * ReadoutSize;
  }
  printf("Sending packets with maximum %u bytes\n", MaxTxSize);

  uint64_t Bytes{0};
  uint64_t TotBytes{0};
  uint64_t Packets{0};
  uint64_t TotPackets{0};

  TSCTimer ReportTimer;
  Timer USClock;

  size_t SentPackets{0}; // counter to determine when to break
  for (;;) {
    if (Settings.NumberOfPackets == 0 || SentPackets < Settings.NumberOfPackets) {
      int readsz = file.read(buffer);
      int BytesToSend = readsz;
      int BytesSent = 0;

      while (BytesToSend > 0 && (Settings.NumberOfPackets == 0 || SentPackets < Settings.NumberOfPackets)) {
        int txsize = BytesToSend >= MaxTxSize ? MaxTxSize : BytesToSend;
        printf("Sending %d bytes\n", txsize);
        DataSource.send(buffer + BytesSent, txsize);
        Bytes += txsize;
        SentPackets++;
        Packets++;
        BytesSent += txsize;
        BytesToSend -= txsize;
      }
    } else {
      std::cout << "Sent " << TotBytes + Bytes << " bytes"
                << " in " << TotPackets + Packets << " packets." << std::endl;
      std::cout << "done" << std::endl;
      exit(0);
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

  return (EXIT_SUCCESS);
}
// GCOVR_EXCL_STOP
