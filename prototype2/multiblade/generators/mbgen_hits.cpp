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
#include <multiblade/caen/DataParser.h>
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

class TxBuffer {
public:
  char * getBuffer() { return Buffer; }

  int getSize() { return DataLength; }

  bool addData(uint32_t time, uint16_t channel, uint16_t adc) {
    //printf("add data %u, %u, %u\n", time, channel, adc);
    Entries++;
    auto hp = (Multiblade::DataParser::Header *)Buffer;
    hp->numElements = Entries;

    auto dataoffset = 32 + (8 * (Entries -1));
    auto dp = (Multiblade::DataParser::ListElement422 *)(Buffer + dataoffset);
    dp->localTime = time;
    dp->channel = channel;
    dp->adcValue = adc;
    DataLength += 8;
    TotalBytes += 8;
    return true;
  }

  bool newPacket(uint64_t global_time, uint32_t digitizer) {
    auto hp = (Multiblade::DataParser::Header *)Buffer;
    memset(Buffer, 0, 32); // clear header
    hp->globalTime = global_time;
    hp->digitizerID = digitizer;
    hp->elementType = Multiblade::ElementType;
    hp->version = Multiblade::Version;
    hp->seqNum = SequenceNumber;
    Entries = 0;
    DataLength = 32;
    TotalBytes += DataLength;
    TotalPackets++;
    SequenceNumber++;
    return true;
  }

  uint64_t TotalPackets{0};
  uint64_t TotalBytes{0};
private:
  int SequenceNumber{0};
  int DataLength{0};
  int Entries{0};
  char Buffer[9000];

};

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

  char RxBuffer[9000];
  TxBuffer txbuffer;

  const int B1M = 1000000;
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(Settings.IpAddress.c_str(), Settings.UDPPort);

  UDPTransmitter DataSource(local, remote);

  ReaderHits file(Settings.FileName);

  int readsz;
  uint64_t tx = 0;
  uint64_t txp = 0;

  TSCTimer report_timer;
  Timer us_clock;

  uint64_t curr_timestamp = 0;
  uint32_t curr_digitizer = 0;

  for (;;) {
    readsz = file.read(RxBuffer);
    if (readsz <= 0) {
      break;
    }

    auto entries = readsz / sizeof(Multiblade::Readout);
    auto mrp = (Multiblade::Readout *)RxBuffer;

    for (unsigned int i = 0; i < entries; i++) {
      if (curr_timestamp != mrp->global_time or curr_digitizer != mrp->digitizer) {
        auto currBufSize = txbuffer.getSize();
        if (currBufSize > 0) {
          if (Settings.NumberOfPackets != 0 && txbuffer.TotalPackets >= Settings.NumberOfPackets) {
            break;
          }
          //printf("Sending packet with size %d\n", currBufSize);
          DataSource.send(txbuffer.getBuffer(), currBufSize);
          tx += currBufSize;
          txp++;
        }

        curr_timestamp = mrp->global_time;
        curr_digitizer = mrp->digitizer;
        txbuffer.newPacket(curr_timestamp, curr_digitizer);
      }

      txbuffer.addData(mrp->local_time, mrp->channel, mrp->adc);
      mrp++;
    }

    if (unlikely((report_timer.timetsc() / TSC_MHZ) >= 1 * 1000000)) {
      auto usecs = us_clock.timeus();

      printf("Tx rate: %8.2f Mbps (%.2f pps), tx %5" PRIu64
             " MB (total: %7" PRIu64 " MB) %" PRIu64 " usecs\n",
             tx * 8.0 / usecs, txp * 1000000.0 / usecs, tx / B1M,
             txbuffer.TotalBytes / B1M, usecs);
      tx = 0;
      txp = 0;
      us_clock.now();
      report_timer.now();
    }
    usleep(Settings.SpeedThrottle * 100);
  }

  // Flush last packet if packet limit has not already been reached
  if (not (Settings.NumberOfPackets != 0 && txbuffer.TotalPackets >= Settings.NumberOfPackets)) {
    auto currBufSize = txbuffer.getSize();
    if (currBufSize > 0) {
      printf("Sending last packet with size %d\n", currBufSize);
      DataSource.send(txbuffer.getBuffer(), currBufSize);
    }
  }

  std::cout << "Sent " << txbuffer.TotalBytes << " bytes"
            << " in " << txbuffer.TotalPackets << " packets." << std::endl;
  std::cout << "done" << std::endl;

  return 0;
}
// GCOVR_EXCL_STOP
