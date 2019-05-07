/** Copyright (C) 2016 - 2019 European Spallation Source ERIC */

#include <CLI/CLI.hpp>
#include <common/TSCTimer.h>
#include <common/Timer.h>

#include <multigrid/generators/ReaderReadouts.h>
#include <gdgem/generators/ReaderHits.h>
#include <gdgem/generators/ReaderReadouts.h>

#include <common/Socket.h>
// GCOVR_EXCL_START

// Non critical but somewhat arbitrary CPU clock speed guess
static constexpr int TscMHz {2900};

constexpr size_t RxBufferSize{9000};

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
  uint64_t PktThrottle{0};
  uint16_t MaxPacketSize{0};
  uint32_t KernelTxBufferSize{1000000};
  uint32_t UpdateIntervalSecs{1};
} Settings;

CLI::App app{"Readout to UDP data generator for Gd-GEM"};

int main(int argc, char *argv[]) {
  app.add_option("-f, --file", Settings.FileName, "Multiblade H5 file with raw readouts");
  app.add_option("-i, --ip", Settings.IpAddress, "Destination IP address");
  app.add_option("-p, --port", Settings.UDPPort, "Destination UDP port");
  app.add_option("-a, --packets", Settings.NumberOfPackets, "Number of packets to send");
  app.add_option("-b, --bytes", Settings.MaxPacketSize, "Maximum number of bytes per packet");
  app.add_option("-t, --throttle", Settings.SpeedThrottle, "Speed throttle (0 is fastest, larger is slower)");
  app.add_option("-s, --pkt_throttle", Settings.PktThrottle, "Extra usleep() after n packets");
  CLI11_PARSE(app, argc, argv);

  if (Settings.FileName.empty()) {
    printf("No input file specified, exiting.\n");
    return (EXIT_FAILURE);
  }

  if (Settings.MaxPacketSize > 9000) { // \todo Should be 8972
    printf("Packet size too large (should be less than 8972)\n");
    return (EXIT_FAILURE);
  }

  hdf5::error::Singleton::instance().auto_print(false);

  char buffer[RxBufferSize];

  const int B1M = 1000000;
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Settings.IpAddress.c_str(), Settings.UDPPort);

  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(Settings.KernelTxBufferSize, 0);
  DataSource.printBufferSizes();

  Settings.FileName = remove_extension(Settings.FileName);

  #ifdef GENERATOR_MULTIGRID_READOUTS
  Multigrid::ReaderReadouts file(Settings.FileName);
  #endif

  #ifdef GENERATOR_GDGEM_READOUTS
  Gem::ReaderReadouts file(Settings.FileName);
  #endif

  #ifdef GENERATOR_GDGEM_HITS
  Gem::ReaderHits file(Settings.FileName);
  #endif

  size_t ReadoutSize = file.getReadoutSize();
  uint16_t MaxTxSize = ReadoutSize * file.getChunkSize();
  assert(MaxTxSize <= RxBufferSize);
  if (Settings.MaxPacketSize != 0) {
    MaxTxSize = (Settings.MaxPacketSize/ReadoutSize) * ReadoutSize;
  }
  std::cout << fmt::format("Sending packets with maximum {} bytes\n", MaxTxSize);

  uint64_t Bytes{0};
  uint64_t TotBytes{0};
  uint64_t Packets{0};
  uint64_t TotPackets{0};

  TSCTimer ReportTimer;
  Timer USClock;

  for (;;) {
    int BytesToSend = file.read(buffer);
    int BytesSent = 0;

    if (BytesToSend > 0 && (Settings.NumberOfPackets == 0 || TotPackets < Settings.NumberOfPackets)) {

      while (BytesToSend > 0 && (Settings.NumberOfPackets == 0 || TotPackets < Settings.NumberOfPackets)) {
        int txsize = BytesToSend >= MaxTxSize ? MaxTxSize : BytesToSend;
        DataSource.send(buffer + BytesSent, txsize);

        BytesSent += txsize;
        BytesToSend -= txsize;
        Bytes += txsize;  // Bytes is periodically cleared
        Packets++; // Packets is periodically cleared
        TotBytes += txsize;
        TotPackets++;
        if (TotPackets % Settings.PktThrottle == 0) {
          usleep(10);
        }
      }
    } else {
      std::cout << fmt::format("Sent {} bytes in {} packets.\n", TotBytes, TotPackets);
      return (EXIT_SUCCESS);
    }

    if (unlikely((ReportTimer.timetsc() / TscMHz) >= Settings.UpdateIntervalSecs * 1000000)) {
      auto usecs = USClock.timeus();
      std::cout << fmt::format("Tx rate: {:8.2f} Mbps ({:.2f} pps), tx {:5} MB (total: {:7} MB) {} usecs\n",
                          Bytes * 8.0 / usecs,
                          Packets * 1000000.0 / usecs,
                          Bytes / B1M,
                          TotBytes / B1M,
                          usecs);
      Bytes = 0;
      Packets = 0;
      USClock.now();
      ReportTimer.now();
    }
    usleep(Settings.SpeedThrottle);
  }
}
// GCOVR_EXCL_STOP
