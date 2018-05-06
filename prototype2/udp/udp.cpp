/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <iostream>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <stdio.h>
#include <unistd.h>

const char *classname = "UDPRaw Detector";

#define TSC_MHZ 3000

/** ----------------------------------------------------- */

class UDPRaw : public Detector {
public:
  UDPRaw(BaseSettings settings);

  ~UDPRaw() { std::cout << "    UDPRaw destroyed" << std::endl; }

  void input_thread();
  const char *detectorname();
};

const char *UDPRaw::detectorname() { return classname; }

UDPRaw::UDPRaw(BaseSettings settings) : Detector("UDPRaw", settings) {
  std::function<void()> inputFunc = [this]() { UDPRaw::input_thread(); };
  AddThreadFunction(inputFunc, "input");
  std::cout << "    UDPRaw created" << std::endl;
}

void UDPRaw::input_thread() {
  uint64_t rx_total = 0;
  uint64_t rx = 0;
  uint64_t rxp = 0;
  const int B1M = 1000000;

  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver raw(local);
  raw.setBufferSizes(4000000, 4000000);
  raw.setRecvTimeout(0, 100000); /// secs, usecs 1/10 s
  raw.printBufferSizes();

  Timer rate_timer;
  TSCTimer report_timer;

  uint32_t seqno = 1;
  uint32_t dropped = 0;
  uint32_t timeseq = 0;
  uint32_t first_dropped = 0;
  while (runThreads) {
    char buffer[10000];
    auto tmprx = raw.receive(buffer, EFUSettings.DetectorRxBufferSize);
    auto tmpseq = *((uint32_t *)buffer);

    if (seqno == tmpseq) {
      seqno++;
    } else {
      // printf("seqno: %u, tmpseq: %u\n", seqno, tmpseq);
      dropped += (tmpseq - seqno);
      seqno = tmpseq + 1;
    }

    if (tmprx > 0) {
      rx += tmprx;
      rxp++;
    }

    if (report_timer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000UL * TSC_MHZ) {
      timeseq++;
      auto usecs = rate_timer.timeus();
      if (timeseq == 2) {
        first_dropped = dropped;
        printf("Recorded %d dropped frames as baseline\n", first_dropped);
      }
      rx_total += rx;
      printf("Rx rate: %.2f Mbps, %.0f pps rx %" PRIu64 " MB (total: %" PRIu64
             " MB) %" PRIu64 " usecs, seq_err %u, PER %.2e\n",
             rx * 8.0 / usecs, rxp * 1000000.0 / usecs, rx / B1M,
             rx_total / B1M, usecs, dropped - first_dropped,
             1.0 * (dropped - first_dropped) / (seqno - first_dropped));
      rx = 0;
      rxp = 0;
      rate_timer.now();
      report_timer.now();
    }
  }
}

/** ----------------------------------------------------- */

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {}

PopulateCLIParser PopulateParser{SetCLIArguments};

class UDPRawFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create(BaseSettings settings) {
    return std::shared_ptr<Detector>(new UDPRaw(settings));
  }
};

UDPRawFactory Factory;
