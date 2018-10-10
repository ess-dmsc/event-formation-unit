/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Processing pipeline for CSPEC instrument (Multi-Grid detector using
/// Mesytec readout)
///
//===----------------------------------------------------------------------===//

#include <multigrid/MGMesytecBase.h>

#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/TimeString.h>
#include <efu/Parser.h>
#include <efu/Server.h>

#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <multigrid/mgmesytec/Sis3153Parser.h>
#include <multigrid/mgmesytec/SequoiaGeometry.h>
//#include <multigrid/mgmesytec/MG24Geometry.h>

#include <multigrid/mgmesytec/EfuMaximum.h>
#include <multigrid/mgmesytec/EfuCenterMass.h>
#include <multigrid/mgmesytec/EfuPrioritized.h>

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this


MGMesytecBase::MGMesytecBase(BaseSettings const &settings, struct MGMesytecSettings & LocalSettings)
       : Detector("CSPEC", settings), MGMesytecSettings(LocalSettings) {
  Stats.setPrefix("efu.mgmesytec");

  LOG(INIT, Sev::Info, "Adding stats");
  // clang-format off
  Stats.create("rx_packets",           mystats.rx_packets);
  Stats.create("rx_bytes",             mystats.rx_bytes);
  Stats.create("sis_discarded_bytes",  mystats.sis_discarded_bytes);
  Stats.create("vmmr_discarded_bytes", mystats.vmmr_discarded_bytes);
  Stats.create("triggers",             mystats.triggers);
  Stats.create("bus_glitches",         mystats.bus_glitches);
  Stats.create("bad_triggers",         mystats.bad_triggers);
  Stats.create("readouts",             mystats.readouts);
  Stats.create("readouts_discarded",   mystats.readouts_discarded);
  Stats.create("readouts_culled",      mystats.readouts_culled);
  Stats.create("geometry_errors",      mystats.geometry_errors);
  Stats.create("timing_errors",        mystats.timing_errors);
  Stats.create("events",               mystats.events);
  Stats.create("tx_bytes",             mystats.tx_bytes);
  // clang-format on

  std::function<void()> inputFunc = [this]() { MGMesytecBase::mainThread(); };
  Detector::AddThreadFunction(inputFunc, "main");

  LOG(INIT, Sev::Info, "Stream monitor data = {}",
      (MGMesytecSettings.monitor ? "YES" : "no"));
  if (!MGMesytecSettings.fileprefix.empty())
    LOG(INIT, Sev::Info, "Dump h5 data in path: {}",
           MGMesytecSettings.fileprefix);
}

///
void MGMesytecBase::init_config()
{
  XTRACE(INIT, INF, "MG Config file: %s", MGMesytecSettings.ConfigFile.c_str());
  mg_config = Multigrid::Config(MGMesytecSettings.ConfigFile);
  XTRACE(INIT, INF, "Multigrid Config\n%s", mg_config.debug().c_str());

  if (MGMesytecSettings.monitor)
    monitor.init(EFUSettings.KafkaBroker, readout_entries);

  if (mg_config.reduction_strategy == "center-mass") {
    mgEfu = std::make_shared<Multigrid::EfuCenterMass>();
  } else if (mg_config.reduction_strategy == "prioritized") {
    mgEfu = std::make_shared<Multigrid::EfuPrioritized>();
  } else {
    mgEfu = std::make_shared<Multigrid::EfuMaximum>();
  }

  mgEfu->mappings = mg_config.mappings;
  mgEfu->hists = monitor.hists;
  mgEfu->raw1 = monitor.readouts;

  if (!MGMesytecSettings.fileprefix.empty())
  {
    dumpfile = Multigrid::HitFile::create(
        MGMesytecSettings.fileprefix + "mgmesytec_" + timeString(), 100);
  }
  vmmr16Parser.spoof_high_time(mg_config.spoof_high_time);
}

void MGMesytecBase::mainThread() {
  init_config();

  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort); //Change name or add more comments
  UDPReceiver cspecdata(local);
  cspecdata.setBufferSizes(EFUSettings.DetectorTxBufferSize, EFUSettings.DetectorRxBufferSize);
  cspecdata.printBufferSizes();
  cspecdata.setRecvTimeout(0, one_tenth_second_usecs); /// secs, usecs

  EV42Serializer ev42serializer(kafka_buffer_size, "multigrid");
  Producer EventProducer(EFUSettings.KafkaBroker, "C-SPEC_detector");
  ev42serializer.setProducerCallback(std::bind(&Producer::produce2<uint8_t>, &EventProducer, std::placeholders::_1));

  Multigrid::Sis3153Parser sis3153parser;
  sis3153parser.buffers.reserve(1000);

  uint8_t buffer[eth_buffer_size];
  int ReadSize {0};
  TSCTimer report_timer;
  for (;;) {
    if ((ReadSize = cspecdata.receive(buffer, eth_buffer_size)) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += ReadSize;
      LOG(PROCESS, Sev::Debug, "Processed UDP packed of size: {}", ReadSize);

      mystats.sis_discarded_bytes += sis3153parser.parse(Buffer<uint8_t>(buffer, ReadSize));

      for (const auto &b : sis3153parser.buffers) {

        mystats.vmmr_discarded_bytes += vmmr16Parser.parse(b);

        if (vmmr16Parser.converted_data.empty())
          continue;

        if (dumpfile) {
          dumpfile->push(vmmr16Parser.converted_data);
        }

        auto parsed_readouts = vmmr16Parser.converted_data.size();

        if (vmmr16Parser.externalTrigger()) {
          parsed_readouts--;
          ev42serializer.pulseTime(RecentPulseTime);
          if (ev42serializer.eventCount())
            mystats.tx_bytes += ev42serializer.produce();
          if (RecentPulseTime) {
            auto PulsePeriod = vmmr16Parser.time() - RecentPulseTime;
            ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
          }
          RecentPulseTime = vmmr16Parser.time();
          HavePulseTime = true;
        }

        mystats.triggers = vmmr16Parser.trigger_count();
        mystats.readouts += parsed_readouts;

        bool bus_glitch = (vmmr16Parser.converted_data.size() > 40);
        if (bus_glitch)
          mystats.bus_glitches++;

        if (mgEfu) {
          mgEfu->ingest(vmmr16Parser.converted_data);

          if (mgEfu->event_good() && !bus_glitch) {
            uint32_t pixel = mg_config.geometry.pixel3D(mgEfu->x(), mgEfu->y(), mgEfu->z());
            uint32_t time = static_cast<uint32_t>(mgEfu->time() - RecentPulseTime);

            XTRACE(PROCESS, DEB, "Event: pixel: %d, time: %d ", pixel, time);
            if (pixel == 0) {
              mystats.geometry_errors++;
            } else if (mgEfu->time() < RecentPulseTime) {
              mystats.timing_errors++;
            } else if (time > (1.00004 * ShortestPulsePeriod)) {
              mystats.timing_errors++;
            } else if (!HavePulseTime) {
              mystats.timing_errors++;
            } else {
              mystats.readouts_culled += (parsed_readouts - mgEfu->used_readouts);
              mystats.events++;
              mystats.tx_bytes += ev42serializer.addEvent(time, pixel);
            }
          } else {
            mystats.readouts_discarded += parsed_readouts;
            if (!vmmr16Parser.externalTrigger())
              mystats.bad_triggers++;
          }
        }
      }
    }

    // Force periodic flushing
    if (report_timer.timetsc() >= EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {
      mystats.tx_bytes += ev42serializer.produce();
      monitor.produce();
      report_timer.now();
    }

    // Checking for exit
    if (not runThreads) {
      // flush anything that remains
      mystats.tx_bytes += ev42serializer.produce();
      monitor.produce();
      LOG(PROCESS, Sev::Info, "Stopping processing thread.");
      return;
    }
  }
}
