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
  char * getBuffer() { return buffer; }

  int getSize() { return DataLength; }

  bool addData(uint32_t time, uint16_t channel, uint16_t adc) {
    //printf("add data %u, %u, %u\n", time, channel, adc);
    Entries++;
    auto hp = (Multiblade::DataParser::Header *)buffer;
    hp->numElements = Entries;

    auto dataoffset = 32 + (8 * (Entries -1));
    auto dp = (Multiblade::DataParser::ListElement422 *)(buffer + dataoffset);
    dp->localTime = time;
    dp->channel = channel;
    dp->adcValue = adc;
    DataLength += 8;
    return true;
  }

  bool newPacket(uint64_t global_time, uint32_t digitizer) {
    auto hp = (Multiblade::DataParser::Header *)buffer;
    memset(buffer, 0, sizeof(buffer));
    hp->globalTime = global_time;
    hp->digitizerID = digitizer;
    hp->elementType = Multiblade::ElementType;
    hp->version = Multiblade::Version;
    hp->seqNum = SequenceNumber;
    Entries = 0;
    DataLength = 32;
    SequenceNumber++;
    return true;
  }
private:

  int SequenceNumber{0};
  int DataLength{0};
  int Entries{0};
  char buffer[9000];
};

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
  TxBuffer txbuffer;

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

  uint64_t curr_timestamp = 0;
  uint32_t curr_digitizer = 0;
  for (;;) {
    if (Settings.NumberOfPackets != 0 && txp_total >= Settings.NumberOfPackets) {
      printf("Max packet reached, exiting...\n");
      break;
    }

    readsz = file.read(buffer);
    if (readsz > 0) {
      auto entries = readsz / sizeof(Multiblade::Readout);
      //printf("\nbuffer contains %lu entries\n", entries);

      auto mrp = (Multiblade::Readout *)buffer;
      for (unsigned int i = 0; i < entries; i++) {

        if (curr_timestamp != mrp->global_time or curr_digitizer != mrp->digitizer) {
          auto currBufSize = txbuffer.getSize();
          if (currBufSize > 0) {
            if (Settings.NumberOfPackets != 0 && txp_total >= Settings.NumberOfPackets) {
              break;
            }
            //printf("Sending packet with size %d\n", currBufSize);
            DataSource.send(txbuffer.getBuffer(), currBufSize);
            tx += currBufSize;
            txp++;
            tx_total += currBufSize;
            txp_total++;
          }

          curr_timestamp = mrp->global_time;
          curr_digitizer = mrp->digitizer;
          txbuffer.newPacket(curr_timestamp, curr_digitizer);
        }

        txbuffer.addData(mrp->local_time, mrp->channel, mrp->adc);
        mrp++;
      }


    } else {
      std::cout << "Sent " << tx_total << " bytes"
                << " in " << txp_total << " packets." << std::endl;
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
