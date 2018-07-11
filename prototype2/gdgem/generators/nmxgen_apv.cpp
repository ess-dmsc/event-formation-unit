/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>

#include <gdgem/generators/NMXArgs.h>
#include <gdgem/generators/ReaderAPV.h>
#include <libs/include/Socket.h>
#include <unistd.h>
/// GCOVR_EXCL_START
const int TSC_MHZ = 2900;

int main(int argc, char *argv[]) {
  NMXArgs opts(argc, argv);
  if (opts.filename.empty())
    return 1;

  hdf5::error::Singleton::instance().auto_print(false);
  //  hdf5::error::auto_print(false);

  char buffer[maxUdpPayloadSize + 100];

  const int B1M = 1000000;
  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);

  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(opts.sndbuf, 0);
  DataSource.printBufferSizes();

  ReaderAPV file(opts.filename);

  int readsz;

  uint64_t tx_total = 0;
  uint64_t txp_total = 0;
  uint64_t tx = 0;
  uint64_t txp = 0;

  TSCTimer report_timer;
  Timer us_clock;

  for (;;) {
    readsz = file.read(buffer);
    if (readsz > 0) {
      DataSource.send(buffer, readsz);
      tx += readsz;
      txp++;
    } else {
      std::cout << "Sent " << tx_total + tx << " bytes"
                << " in " << txp_total + txp << " packets." << std::endl;
      std::cout << "done" << std::endl;
      exit(0);
    }

    if (unlikely((report_timer.timetsc() / TSC_MHZ) >= opts.updint * 1000000)) {
      auto usecs = us_clock.timeus();
      tx_total += tx;
      txp_total += txp;
      printf("Tx rate: %8.2f Mbps (%.2f pps), tx %5" PRIu64
             " MB (total: %7" PRIu64 " MB) %" PRIu64 " usecs\n",
             tx * 8.0 / usecs, txp * 1000000.0 / usecs, tx / B1M,
             tx_total / B1M, usecs);
      tx = 0;
      txp = 0;
      us_clock.now();
      report_timer.now();
    }
    usleep(opts.throttle * 1000);
  }

  return 0;
}
/// GCOVR_EXCL_STOP
