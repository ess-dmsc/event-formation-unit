/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief CSPEC Detector implementation
 */

#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <multigrid/mgcncs/CalibrationFile.h>
#include <multigrid/mgcncs/ChanConv.h>
#include <multigrid/mgcncs/DataParser.h>
#include <multigrid/mgcncs/MultigridGeometry.h>
//#include <cspec/CSPECEvent.h>
#include <cstring>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <mutex>
#include <queue>
#include <stdio.h>
#include <unistd.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_CRI

using namespace memory_sequential_consistent; // Lock free fifo

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this

/** ----------------------------------------------------- */

class CSPEC : public Detector {
public:
  CSPEC(BaseSettings settings);
  void input_thread();
  void processing_thread();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 1000;
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 1000000;

  int LoadCalib(std::vector<std::string> cmdargs, UNUSED char *output,
                UNUSED unsigned int *obytes);
  int ShowCalib(std::vector<std::string> cmdargs, UNUSED char *output,
                UNUSED unsigned int *obytes);

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  std::mutex eventq_mutex, cout_mutex;

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo_push_errors;
    int64_t pad_a[5]; /**< @todo check alignment*/

    // Processing Counters
    int64_t rx_readouts;
    int64_t rx_error_bytes;
    int64_t rx_discards;
    int64_t rx_idle1;
    int64_t geometry_errors;
    int64_t fifo_seq_errors;
    // Output Counters
    int64_t rx_events;
    int64_t tx_bytes;
  } ALIGN(64) mystats;

  std::atomic_bool NewCalibrationData{false};
  uint16_t wirecal[CSPECChanConv::adcsize];
  uint16_t gridcal[CSPECChanConv::adcsize];
};

struct DetectorSettingsStruct {
  std::string fileprefix{""};
} DetectorSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("--dumptofile", DetectorSettings.fileprefix,
                    "dump to specified file")->group("MGCNCS");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

//=============================================================================
int CSPEC::LoadCalib(std::vector<std::string> cmdargs,
                     __attribute__((unused)) char *output,
                     __attribute__((unused)) unsigned int *obytes) {
  XTRACE(CMD, INF, "CSPEC_LOAD_CALIB\n");
  GLOG_INF("CSPEC_LOAD_CALIB");
  if (cmdargs.size() != 2) {
    XTRACE(CMD, WAR, "CSPEC_LOAD_CALIB: wrong number of arguments\n");
    return -Parser::EBADARGS;
  }
  CalibrationFile calibfile;
  auto ret = calibfile.load(cmdargs.at(1), (char *)wirecal, (char *)gridcal);
  if (ret < 0) {
    return -Parser::EBADARGS;
  }
  NewCalibrationData = true;

  return Parser::OK;
}

//=============================================================================
int CSPEC::ShowCalib(std::vector<std::string> cmdargs, char *output,
                     unsigned int *obytes) {
  auto nargs = cmdargs.size();
  unsigned int offset = 0;
  XTRACE(CMD, INF, "CSPEC_SHOW_CALIB\n");
  GLOG_INF("CSPEC_SHOW_CALIB");
  if (nargs == 1) {
    offset = 0;
  } else if (nargs == 2) {
    offset = atoi(cmdargs.at(1).c_str());
  } else {
    XTRACE(CMD, WAR, "CSPEC_SHOW_CALIB: wrong number of arguments\n");
    return -Parser::EBADARGS;
  }

  if (offset > CSPECChanConv::adcsize - 1) {
    return -Parser::EBADARGS;
  }

  *obytes =
      snprintf(output, SERVER_BUFFER_SIZE, "wire %d 0x%04x, grid %d 0x%04x",
               offset, wirecal[offset], offset, gridcal[offset]);

  return Parser::OK;
}

CSPEC::CSPEC(BaseSettings settings) : Detector("CSPEC Detector (2 thread pipeline)", settings) {
  Stats.setPrefix("efu.cspec2");

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
  Stats.create("input.rx_packets",                mystats.rx_packets);
  Stats.create("input.rx_bytes",                  mystats.rx_bytes);
  Stats.create("input.i2pfifo_dropped",           mystats.fifo_push_errors);
  Stats.create("processing.rx_readouts",          mystats.rx_readouts);
  Stats.create("processing.rx_error_bytes",       mystats.rx_error_bytes);
  Stats.create("processing.rx_discards",          mystats.rx_discards);
  Stats.create("processing.rx_idle",              mystats.rx_idle1);
  Stats.create("processing.rx_geometry_errors",   mystats.geometry_errors);
  Stats.create("processing.fifo_seq_errors",      mystats.fifo_seq_errors);
  Stats.create("output.rx_events",                mystats.rx_events);
  Stats.create("output.tx_bytes",                 mystats.tx_bytes);
  // clang-format on

  std::function<void()> inputFunc = [this]() { CSPEC::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    CSPEC::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  AddCommandFunction("CSPEC_LOAD_CALIB",
                     [this](std::vector<std::string> cmdargs, char *output,
                            unsigned int *obytes) {
                       return CSPEC::LoadCalib(cmdargs, output, obytes);
                     });
  AddCommandFunction("CSPEC_SHOW_CALIB",
                     [this](std::vector<std::string> cmdargs, char *output,
                            unsigned int *obytes) {
                       return CSPEC::ShowCalib(cmdargs, output, obytes);
                     });

  XTRACE(INIT, ALW, "Creating %d Ethernet ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries + 11);
}

void CSPEC::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver cspecdata(local);
  cspecdata.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  cspecdata.printBufferSizes();
  cspecdata.setRecvTimeout(0, 100000); // secs, usecs, One tenth of a second

  int rdsize;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getDataIndex();

    /** this is the processing step */
    eth_ringbuf->setDataLength(eth_index, 0);
    if ((rdsize = cspecdata.receive(eth_ringbuf->getDataBuffer(eth_index),
                                    eth_ringbuf->getMaxBufSize())) > 0) {
      eth_ringbuf->setDataLength(eth_index, rdsize);
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;
      XTRACE(INPUT, DEB, "rdsize: %u\n", rdsize);

      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo_push_errors++;
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

void CSPEC::processing_thread() {
  CSPECChanConv conv;

  Producer producer(EFUSettings.KafkaBroker, "C-SPEC_detector");
  FBSerializer flatbuffer(kafka_buffer_size, producer);

  MultiGridGeometry geom(1, 2, 48, 4, 16);

  CSPECData dat(250, &conv, &geom, DetectorSettings.fileprefix); // Default signal thresholds

  TSCTimer report_timer;
  TSCTimer timestamp;

  unsigned int data_index;
  while (1) {
    // Check for control from mothership (main)
    if (NewCalibrationData) {
      XTRACE(PROCESS, INF, "processing_thread loading new calibrations\n");
      conv.load_calibration(wirecal, gridcal);
      NewCalibrationData = false;
    }

    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;
      usleep(1);
    } else {
      auto len = eth_ringbuf->getDataLength(data_index);
      if (len == 0) {
        mystats.fifo_seq_errors++;
      } else {
        dat.receive(eth_ringbuf->getDataBuffer(data_index),
                    eth_ringbuf->getDataLength(data_index));
        mystats.rx_readouts += dat.elems;
        mystats.rx_error_bytes += dat.error;
        mystats.rx_discards += dat.input_filter();

        for (unsigned int id = 0; id < dat.elems; id++) {
          auto d = dat.data[id];
          if (d.valid) {
            uint32_t tmptime;
            uint32_t tmppixel;
            if (dat.createevent(d, &tmptime, &tmppixel) < 0) {
              mystats.geometry_errors++;
              assert(mystats.geometry_errors <= mystats.rx_readouts);
            } else {
              mystats.tx_bytes += flatbuffer.addevent(tmptime, tmppixel);
              mystats.rx_events++;
            }
          }
        }
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {
      mystats.tx_bytes += flatbuffer.produce();
      report_timer.now();
    }

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping processing thread.\n");
      return;
    }
  }
}

/** ----------------------------------------------------- */

class CSPECFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create(BaseSettings settings) {
    return std::shared_ptr<Detector>(new CSPEC(settings));
  }
};

CSPECFactory Factory;
