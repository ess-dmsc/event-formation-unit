//
// Created by soegaard on 8/22/17.
//

#include <cinttypes>
#include <unistd.h>
#include <dataformats/multigrid/inc/DataSave.h>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/ESSGeometry.h>
#include <common/FBSerializer.h>
#include <common/NewStats.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>

#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>

#include <mbcaen/MB16Detector.h>
#include <mbcaen/MBData.h>
#include <mbcommon/multiBladeEventBuilder.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "Multiblade detector with CAEN readout";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class MBCAEN : public Detector {
public:
  MBCAEN(void *args);
  void input_thread();
  void processing_thread();

  int statsize();
  int64_t statvalue(size_t index);
  std::string &statname(size_t index);
  const char *detectorname();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 1000;
  static const int eth_buffer_size = 1600;
  static const int kafka_buffer_size = 1000000;

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  NewStats ns{"efu2.mbcaen."};

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo1_push_errors;
    int64_t pad[5];

    // Processing Counters
    int64_t rx_idle1;
    int64_t rx_readouts;
    int64_t tx_bytes;
    int64_t rx_events;
    int64_t geometry_errors;
    int64_t fifo_seq_errors;
  } ALIGN(64) mystats;

  EFUArgs *opts;
};

MBCAEN::MBCAEN(void *args) {
  opts = (EFUArgs *)args;

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
    ns.create("input.rx_packets",                &mystats.rx_packets);
    ns.create("input.rx_bytes",                  &mystats.rx_bytes);
    ns.create("input.fifo1_push_errors",         &mystats.fifo1_push_errors);
    ns.create("processing.rx_readouts",          &mystats.rx_readouts);
    ns.create("processing.rx_idle1",             &mystats.rx_idle1);
    ns.create("processing.tx_bytes",             &mystats.tx_bytes);
    ns.create("processing.rx_events",            &mystats.rx_events);
    ns.create("processing.rx_geometry_errors",   &mystats.geometry_errors);
    ns.create("processing.fifo_seq_errors",      &mystats.fifo_seq_errors);
  // clang-format on

  XTRACE(INIT, ALW, "Creating %d Multiblade Rx ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries + 11); // @todo workaround
  assert(eth_ringbuf != 0);
}

int MBCAEN::statsize() { return ns.size(); }

int64_t MBCAEN::statvalue(size_t index) { return ns.value(index); }

std::string &MBCAEN::statname(size_t index) { return ns.name(index); }

const char *MBCAEN::detectorname() { return classname; }

void MBCAEN::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(opts->ip_addr.c_str(), opts->port);
  UDPServer mbdata(local);
  mbdata.buflen(opts->buflen);
  mbdata.setbuffers(0, opts->rcvbuf);
  mbdata.printbuffers();
  mbdata.settimeout(0, 100000); // One tenth of a second

  int rdsize;
  TSCTimer report_timer;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getindex();

    /** this is the processing step */
    eth_ringbuf->setdatalength(eth_index, 0);
    if ((rdsize = mbdata.receive(eth_ringbuf->getdatabuffer(eth_index),
                                 eth_ringbuf->getmaxbufsize())) > 0) {
      eth_ringbuf->setdatalength(eth_index, rdsize);
      XTRACE(PROCESS, DEB, "Received an udp packet of length %d bytes\n", rdsize);
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;

      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo1_push_errors++;
      } else {
        eth_ringbuf->nextbuffer();
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {

      if (opts->proc_cmd == opts->thread_cmd::THREAD_TERMINATE) {
        XTRACE(INPUT, ALW, "Stopping input thread - stopcmd: %d\n",
               opts->proc_cmd);
        return;
      }

      report_timer.now();
    }
  }
}

void MBCAEN::processing_thread() {
  const uint32_t ncass = 6;
  uint8_t nwires = 32;
  uint8_t nstrips = 32;

  #ifdef DUMPTOFILE // only active if cmake -DDUMPTOFILE=ON
  DataSave mbdatasave{"multiblade_", 100000000};
  mbdatasave.tofile("# time, digitizer, channel, adc\n");
  #endif

  ESSGeometry essgeom(nstrips, ncass * nwires, 1, 1);
  MB16Detector mb16;
  Producer eventprod(opts->broker, "MB_detector");
  FBSerializer flatbuffer(kafka_buffer_size, eventprod);

  multiBladeEventBuilder builder[ncass];
  for (uint32_t i = 0; i < ncass; i++) {
    builder[i].setNumberOfWireChannels(nwires);
    builder[i].setNumberOfStripChannels(nstrips);
  }

  MBData mbdata;

  unsigned int data_index;
  TSCTimer report_timer;
  while (1) {
    if ((input2proc_fifo.pop(data_index)) == false) {
      // There is NO data in the FIFO - do stop checks and sleep a little
      mystats.rx_idle1++;
      // Checking for exit
      if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {

        mystats.tx_bytes += flatbuffer.produce();

        if (opts->proc_cmd == opts->thread_cmd::THREAD_TERMINATE) {
          XTRACE(INPUT, ALW, "Stopping processing thread - stopcmd: %d\n",
                 opts->proc_cmd);
          return;
        }
        report_timer.now();
      }
      usleep(10);

    } else { // There is data in the FIFO - do processing
      auto datalen = eth_ringbuf->getdatalength(data_index);
      if (datalen == 0) {
        mystats.fifo_seq_errors++;
      } else {
        auto dataptr = eth_ringbuf->getdatabuffer(data_index);
        mbdata.recieve(dataptr, datalen);

        auto dat = mbdata.data;
        mystats.rx_readouts += dat.size();

        for (uint i = 0; i < dat.size(); i++) {

          auto dp = dat.at(i);

          // @todo fixme remove - is an artifact of mbtext2udp
          if (dp.digi == UINT8_MAX && dp.chan == UINT8_MAX &&
              dp.adc == UINT16_MAX && dp.time == UINT32_MAX) {
            XTRACE(PROCESS, DEB, "Last point\n");
            builder[0].lastPoint();
            break;
          }

          auto cassette = mb16.cassette(dp.digi);
          if (cassette < 0) {
            break;
          }

          #ifdef DUMPTOFILE
          mbdatasave.tofile("%d,%d,%d,%d\n", dp.time, dp.digi, dp.chan, dp.adc);
          #endif

          if (builder[cassette].addDataPoint(dp.chan, dp.adc, dp.time)) {
            auto xcoord = builder[cassette].getStripPosition() - 32; // pos 32 - 63
            auto ycoord = cassette * nwires + builder[cassette].getWirePosition(); // pos 0 - 31

            uint32_t pixel_id = essgeom.pixelSP2D(xcoord, ycoord);

            XTRACE(PROCESS, DEB,
                   "digi: %d, wire: %d, strip: %d, x: %d, y:%d, pixel_id: %d\n",
                   dp.digi, (int)xcoord, (int)ycoord,
                   (int)builder[cassette].getWirePosition(),
                   (int)builder[cassette].getStripPosition(), pixel_id);

            if (pixel_id == 0) {
              mystats.geometry_errors++;
            } else {
              mystats.tx_bytes += flatbuffer.addevent(builder[cassette].getTimeStamp(), pixel_id);
              mystats.rx_events++;
            }
          }
        }
      }
    }
  }
}

/** ----------------------------------------------------- */

class MBCAENFactory : DetectorFactory {
public:
  std::shared_ptr<Detector> create(void *args) {
    return std::shared_ptr<Detector>(new MBCAEN(args));
  }
};

MBCAENFactory Factory;
