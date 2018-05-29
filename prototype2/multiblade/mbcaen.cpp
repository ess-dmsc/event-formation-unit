//
// Created by soegaard on 8/22/17.
//

#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <common/DataSave.h>
#include <unistd.h>

#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>

#include <mbcaen/MB16Detector.h>
#include <mbcaen/MBData.h>
#include <mbcommon/multiBladeEventBuilder.h>

#include <logical_geometry/ESSGeometry.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "Multiblade detector with CAEN readout";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class MBCAEN : public Detector {
public:
  MBCAEN(BaseSettings settings);
  void input_thread();
  void processing_thread();

  const char *detectorname();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 1000;
  static const int eth_buffer_size = 1600;
  static const int kafka_buffer_size = 1000000;

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

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
};

struct DetectorSettingsStruct {
  std::string fileprefix{""};
} DetectorSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("--dumptofile", DetectorSettings.fileprefix,
                    "dump to specified file")->group("MBCAEN");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

MBCAEN::MBCAEN(BaseSettings settings) : Detector("MBCAEN", settings) {
  Stats.setPrefix("efu.mbcaen");

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
    Stats.create("input.rx_packets",                mystats.rx_packets);
    Stats.create("input.rx_bytes",                  mystats.rx_bytes);
    Stats.create("input.fifo1_push_errors",         mystats.fifo1_push_errors);
    Stats.create("processing.rx_readouts",          mystats.rx_readouts);
    Stats.create("processing.rx_idle1",             mystats.rx_idle1);
    Stats.create("processing.tx_bytes",             mystats.tx_bytes);
    Stats.create("processing.rx_events",            mystats.rx_events);
    Stats.create("processing.rx_geometry_errors",   mystats.geometry_errors);
    Stats.create("processing.fifo_seq_errors",      mystats.fifo_seq_errors);
  // clang-format on

  std::function<void()> inputFunc = [this]() { MBCAEN::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    MBCAEN::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Multiblade Rx ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries +
                                                11); // @todo workaround
  assert(eth_ringbuf != 0);
}

const char *MBCAEN::detectorname() { return classname; }

void MBCAEN::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver mbdata(local);
  // mbdata.buflen(opts->buflen);
  mbdata.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  mbdata.printBufferSizes();
  mbdata.setRecvTimeout(0, 100000); /// secs, usecs 1/10s

  int rdsize;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getDataIndex();

    /** this is the processing step */
    eth_ringbuf->setDataLength(eth_index, 0);
    if ((rdsize = mbdata.receive(eth_ringbuf->getDataBuffer(eth_index),
                                 eth_ringbuf->getMaxBufSize())) > 0) {
      eth_ringbuf->setDataLength(eth_index, rdsize);
      XTRACE(PROCESS, DEB, "Received an udp packet of length %d bytes\n",
             rdsize);
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;

      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo1_push_errors++;
      } else {
        eth_ringbuf->getNextBuffer();
      }
    }

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.\n");
      return;
    }
  }
}

void MBCAEN::processing_thread() {
  const uint32_t ncass = 6;
  uint8_t nwires = 32;
  uint8_t nstrips = 32;

  std::shared_ptr<DataSave> mbdatasave;
  bool dumptofile = !DetectorSettings.fileprefix.empty();
  if (dumptofile)
  {
    mbdatasave = std::make_shared<DataSave>(DetectorSettings.fileprefix + "_multiblade_", 100000000);
    mbdatasave->tofile("# time, digitizer, channel, adc\n");
  }

  ESSGeometry essgeom(nstrips, ncass * nwires, 1, 1);
  MB16Detector mb16;

  Producer eventprod(EFUSettings.KafkaBroker, "MB_detector");
  FBSerializer flatbuffer(kafka_buffer_size, eventprod);

  multiBladeEventBuilder builder[ncass];
  for (uint32_t i = 0; i < ncass; i++) {
    builder[i].setNumberOfWireChannels(nwires);
    builder[i].setNumberOfStripChannels(nstrips);
  }

  MBData mbdata;

  unsigned int data_index;
  TSCTimer produce_timer;
  while (1) {
    if ((input2proc_fifo.pop(data_index)) == false) {
      // There is NO data in the FIFO - do stop checks and sleep a little
      mystats.rx_idle1++;
      // Checking for exit
      if (produce_timer.timetsc() >=
          EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {

        mystats.tx_bytes += flatbuffer.produce();

        if (not runThreads) {
          XTRACE(INPUT, ALW, "Stopping processing thread.\n");
          return;
        }

        produce_timer.now();
      }
      usleep(10);

    } else { // There is data in the FIFO - do processing
      auto datalen = eth_ringbuf->getDataLength(data_index);
      if (datalen == 0) {
        mystats.fifo_seq_errors++;
      } else {
        auto dataptr = eth_ringbuf->getDataBuffer(data_index);
        mbdata.receive(dataptr, datalen);

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

          if (dumptofile) {
            mbdatasave->tofile("%d,%d,%d,%d\n", dp.time, dp.digi, dp.chan, dp.adc);
          }

          if (builder[cassette].addDataPoint(dp.chan, dp.adc, dp.time)) {
            auto xcoord =
                builder[cassette].getStripPosition() - 32; // pos 32 - 63
            auto ycoord = cassette * nwires +
                          builder[cassette].getWirePosition(); // pos 0 - 31

            uint32_t pixel_id = essgeom.pixel2D(xcoord, ycoord);

            XTRACE(PROCESS, DEB,
                   "digi: %d, wire: %d, strip: %d, x: %d, y:%d, pixel_id: %d\n",
                   dp.digi, (int)xcoord, (int)ycoord,
                   (int)builder[cassette].getWirePosition(),
                   (int)builder[cassette].getStripPosition(), pixel_id);

            if (pixel_id == 0) {
              mystats.geometry_errors++;
            } else {
              mystats.tx_bytes += flatbuffer.addevent(
                  builder[cassette].getTimeStamp(), pixel_id);
              mystats.rx_events++;
            }
          }
        }
      }
    }
  }
}

/** ----------------------------------------------------- */

DetectorFactory<MBCAEN> Factory;
