/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <libs/include/Socket.h>
#include <nmxgen/EventNMX.h>
#include <nmxgen/NMXArgs.h>
#include <nmxgen/ParserClusterer.h>
#include <nmxgen/ReaderVMM.h>
#include <unistd.h>

// const int TSC_MHZ = 2900;

int main(int argc, char *argv[]) {
  NMXArgs opts(argc, argv);
  H5::Exception::dontPrint();

  char buffer[9000];

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);

  UDPClient DataSource(local, remote);
  DataSource.buflen(opts.buflen);
  DataSource.setbuffers(opts.sndbuf, 0);
  DataSource.printbuffers();

  ReaderVMM file(opts.filename);

  int readsz;
  uint64_t pkt = 0;
  uint64_t bytes = 0;

  HistMap2D image;

  ParserClusterer parser;

  while ((pkt < opts.txPkt) && ((readsz = file.read(buffer)) > 0)) {
    parser.parse(buffer, readsz);

    DataSource.send(buffer, readsz);

    while (parser.event_ready()) {
      auto event = parser.get();
      event.analyze(true, 3, 7);
      if (event.good)
        image[c2d(static_cast<uint32_t>(event.x.center),
                  static_cast<uint32_t>(event.y.center))]++;
    }

    bytes += readsz;
    pkt++;

    // usleep(opts.speed_level * 1000);
  }

  printf("Sent: %" PRIu64 " packets\n", pkt);
  printf("Sent: %" PRIu64 " bytes\n", bytes);
  printf("Image: %" PRIu64 " points\n", (uint64_t)image.size());

  if (opts.outfile.empty() || opts.filename.empty())
    return 0;

  auto ofile = H5CC::File(opts.outfile, H5CC::Access::rw_truncate);
  auto grp = ofile.require_group("images");

  printf("Success creating\n");

  write(grp, "file", image, 1);

  return 0;
}
