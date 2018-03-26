
/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <sonde/ideas/Data.h>
#include <stdio.h>
#include <unistd.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "SoNDe detector using IDEA readout";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class SONDEIDEA : public Detector {
public:
  SONDEIDEA(BaseSettings settings);
  ~SONDEIDEA() { printf("sonde destructor called\n"); }

  void input_thread();
  void processing_thread();

  const char *detectorname();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 20000;
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 124000; /**< events */

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  NewStats ns{"efu.sonde."}; //

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo_push_errors;
    int64_t rx_pktlen_0;
    int64_t pad[4];

    // Processing and Output counters
    int64_t rx_idle1;
    int64_t rx_events;
    int64_t rx_geometry_errors;
    int64_t tx_bytes;
    int64_t rx_seq_errors;
    int64_t fifo_synch_errors;
  } ALIGN(64) mystats;
};

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {}

PopulateCLIParser PopulateParser{SetCLIArguments};

SONDEIDEA::SONDEIDEA(BaseSettings settings) : Detector("SoNDe detector using IDEA readout", settings) {
  Stats.setPrefix("efu.sonde");

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("input.rx_packets",                mystats.rx_packets);
  Stats.create("input.rx_bytes",                  mystats.rx_bytes);
  Stats.create("input.dropped",                   mystats.fifo_push_errors);
  Stats.create("input.rx_seq_errors",             mystats.rx_seq_errors);
  Stats.create("processing.idle",                 mystats.rx_idle1);
  Stats.create("processing.rx_events",            mystats.rx_events);
  Stats.create("processing.rx_geometry_errors",   mystats.rx_geometry_errors);
  Stats.create("processing.rx_seq_errors",        mystats.rx_seq_errors);
  Stats.create("output.tx_bytes",                 mystats.tx_bytes);
  // clang-format on
  std::function<void()> inputFunc = [this]() { SONDEIDEA::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    SONDEIDEA::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d SONDE Rx ringbuffers of size %d",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(
      eth_buffer_max_entries + 1); /** @todo testing workaround */
  assert(eth_ringbuf != 0);
}

const char *SONDEIDEA::detectorname() { return classname; }

void SONDEIDEA::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPServer sondedata(local);
  sondedata.setbuffers(0, EFUSettings.DetectorRxBufferSize);
  sondedata.printbuffers();
  sondedata.settimeout(0, 100000); // 1/10 second

  int rdsize;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getindex();

    /** this is the processing step */
    eth_ringbuf->setdatalength(eth_index, 0);
    if ((rdsize = sondedata.receive(eth_ringbuf->getdatabuffer(eth_index),
                                    eth_ringbuf->getmaxbufsize())) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;
      eth_ringbuf->setdatalength(eth_index, rdsize);

      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo_push_errors++;
      } else {
        eth_ringbuf->nextbuffer();
      }
    }

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.");
      return;
    }
  }
}

void SONDEIDEA::processing_thread() {
  SoNDeGeometry geometry;
  IDEASData ideasdata(&geometry);
  Producer eventprod(EFUSettings.KafkaBroker, "SKADI_detector");
  FBSerializer flatbuffer(kafka_buffer_size, eventprod);

  unsigned int data_index;

  TSCTimer produce_timer;
  while (1) {
    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;
      usleep(10);

    } else {

      auto len = eth_ringbuf->getdatalength(data_index);
      if (len == 0) {
        mystats.fifo_synch_errors++;
      } else {
        int events =
            ideasdata.parse_buffer(eth_ringbuf->getdatabuffer(data_index), len);

        mystats.rx_geometry_errors += ideasdata.errors;
        mystats.rx_events += ideasdata.events;
        mystats.rx_seq_errors = ideasdata.ctr_outof_sequence;

        if (events > 0) {
          for (int i = 0; i < events; i++) {
            XTRACE(PROCESS, DEB, "flatbuffer.addevent[i: %d](t: %d, pix: %d)",
                   i, ideasdata.data[i].time, ideasdata.data[i].pixel_id);
            mystats.tx_bytes += flatbuffer.addevent(ideasdata.data[i].time,
                                                    ideasdata.data[i].pixel_id);
          }
        }

        if (produce_timer.timetsc() >=
            EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {
          mystats.tx_bytes += flatbuffer.produce();
          produce_timer.now();
        }
      }
    }
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.");
      return;
    }
  }
}

/** ----------------------------------------------------- */

class SONDEIDEAFactory : DetectorFactory {
public:
  std::shared_ptr<Detector> create(BaseSettings settings) {
    return std::shared_ptr<Detector>(new SONDEIDEA(settings));
  }
};

SONDEIDEAFactory Factory;
