/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dataformats/multigrid/inc/RunSpec.h>
#include <dataformats/multigrid/inc/RunSpecParse.h>
#include <iomanip>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <multigrid/mgcncsgen/JsonArgs.h>
#include <sstream>
#include <unistd.h>

int main(int argc, char *argv[]) {
  JsonArgs opts(argc, argv);

  char buffer[9000];
  const int readoutdatasize = 40;

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts.dest_ip.c_str(), opts.port);

  UDPTransmitter DataSource(local, remote);
  DataSource.setBufferSizes(opts.sndbuf, 0);
  DataSource.printBufferSizes();

  std::vector<RunSpec *> runs;
  if (!opts.runfile.empty()) { /**< get config from json file */
    RunSpecParse runspecfile(opts.runfile);

    /** use all files in runfile:runspecification, output dir does not apply */
    runs = runspecfile.getruns(opts.runspecification, opts.basedir, "", 0, 0);
  }

  uint64_t pkt = 0;
  uint64_t bytes = 0;
  TSCTimer throttle_timer;

  for (auto run : runs) {
    for (unsigned int j = run->start_; j <= run->end_; j++) {
      std::ostringstream seqno;
      seqno.width(3);
      seqno << std::setfill('0') << j;
      std::string file = run->dir_ + run->prefix_ + seqno.str() + run->postfix_;
      printf("Streaming file %s\n", file.c_str());

      FILE *f = fopen(file.c_str(), "r");
      if (f == NULL) {
        printf("error: cannot open file \'%s\'\n", file.c_str());
        return -1;
      }

      unsigned int readsize = (opts.buflen / readoutdatasize) * readoutdatasize;
      assert(readsize <= 9000);
      int readsz;

      while ((readsz = fread(buffer, 1, readsize, f)) > 0) {
        DataSource.send(buffer, readsz);
        bytes += readsz;
        pkt++;

        usleep(opts.speed_level * 1000);
      }

      fclose(f);
      f = NULL;
    }
  }

  printf("Sent: %" PRIu64 " packets\n", pkt);
  printf("Sent: %" PRIu64 " bytes\n", bytes);

  return 0;
}
