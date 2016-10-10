/** Copyright (C) 2016 European Spallation Source */

#include <Detector.h>
#include <EFUArgs.h>
#include <Socket.h>
#include <Timer.h>
#include <cassert>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace std;
const char *classname = "UDPRaw Object";

class UDPRawTx : public Detector {
public:
  void input_thread(void *a);
  UDPRawTx() { cout << "    UDPRawTx created" << endl; };
  ~UDPRawTx() { cout << "    UDPRawTx destroyed" << endl; };
};

void UDPRawTx::input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

  uint64_t tx_total = 0;
  uint64_t tx = 0;
  uint64_t txp = 0;

  const int B1M = 1000000;

  Socket::Endpoint local("0.0.0.0", 0);
  Socket::Endpoint remote(opts->dest_ip.c_str(), opts->port);
  UDPClient udptx(local, remote);
  udptx.buflen(opts->buflen);
  udptx.printbuffers();
  udptx.setbuffers(0, opts->sndbuf);
  udptx.printbuffers();

  Timer upd;
  auto usecs = upd.timeus();

  for (;;) {
    tx += udptx.send();

    if (tx > 0)
      txp++;

    if ((txp % 100) == 0)
      usecs = upd.timeus();

    if (usecs >= 1000000) {
      tx_total += tx;
      printf("Tx rate: %.2f Mbps, tx %" PRIu64 " MB (total: %" PRIu64
             " MB) %ld usecs\n",
             tx * 8.0 / (usecs / 1000000.0) / B1M, tx / B1M, tx_total / B1M,
             usecs);
      tx = 0;
      upd.now();
      usecs = upd.timeus();
    }
  }
}

/** */
class UDPRawTxFactory : public DetectorFactory {
public:
  Detector *create() {
    cout << "    making UDPRawTx" << endl;
    return new UDPRawTx;
  }
};

UDPRawTxFactory Factory;
