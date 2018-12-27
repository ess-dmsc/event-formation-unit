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

#include <multigrid/geometry/SequoiaGeometry.h>
//#include <multigrid/geometry/MG24Geometry.h>

#include <multigrid/reduction/EfuMaximum.h>
#include <multigrid/reduction/EfuCenterMass.h>
#include <multigrid/reduction/EfuPrioritized.h>

#include <common/Trace.h>
#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
//#undef TRC_MASK
//#define TRC_MASK 0

// \todo MJC's workstation - not reliable
static constexpr int TscMHz {2900};

MGMesytecBase::MGMesytecBase(BaseSettings const &settings, struct MGMesytecSettings & LocalSettings)
       : Detector("CSPEC", settings), MGMesytecSettings(LocalSettings) {
  Stats.setPrefix("efu.mgmesytec");

  LOG(INIT, Sev::Info, "Adding stats");
  // clang-format off
  Stats.create("rx_packets",           mystats.rx_packets);
  Stats.create("rx_bytes",             mystats.rx_bytes);
  Stats.create("rx_discarded_bytes",   mystats.discarded_bytes);
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

  LOG(INIT, Sev::Info, "Stream monitor data = {}",
         (MGMesytecSettings.monitor ? "YES" : "no"));
  if (!MGMesytecSettings.FilePrefix.empty())
    LOG(INIT, Sev::Info, "Dump h5 data in path: {}",
           MGMesytecSettings.FilePrefix);

  std::function<void()> inputFunc = [this]() { MGMesytecBase::mainThread(); };
  Detector::AddThreadFunction(inputFunc, "main");
}

///
void MGMesytecBase::init_config()
{
  LOG(INIT, Sev::Info, "MG Config file: {}", MGMesytecSettings.ConfigFile);
  mg_config = Multigrid::Config(MGMesytecSettings.ConfigFile);
  LOG(INIT, Sev::Info, "Multigrid Config\n{}", mg_config.debug());

  if (MGMesytecSettings.monitor)
    monitor.init(EFUSettings.KafkaBroker, readout_entries);

//  if (mg_config.reduction_strategy == "center-mass") {
//    mgEfu = std::make_shared<Multigrid::EfuCenterMass>();
//  } else if (mg_config.reduction_strategy == "prioritized") {
//    mgEfu = std::make_shared<Multigrid::EfuPrioritized>();
//  } else {
//    mgEfu = std::make_shared<Multigrid::EfuMaximum>();
//  }
//
//  mgEfu->mappings = mg_config.mappings;
//  mgEfu->hists = monitor.hists;
//  mgEfu->raw1 = monitor.readouts;

//  if (!MGMesytecSettings.FilePrefix.empty())
//  {
//    dumpfile = Multigrid::ReadoutFile::create(
//        MGMesytecSettings.FilePrefix + "mgmesytec_" + timeString(), 100);
//  }
//  vmmr16Parser.spoof_high_time(mg_config.spoof_high_time);
}

void MGMesytecBase::mainThread() {
  init_config();

  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort); //Change name or add more comments
  XTRACE(INIT, DEB, "server: %s, port %d", EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort);
  UDPReceiver cspecdata(local);
  cspecdata.setBufferSizes(EFUSettings.DetectorTxBufferSize, EFUSettings.DetectorRxBufferSize);
  cspecdata.printBufferSizes();
  cspecdata.setRecvTimeout(0, one_tenth_second_usecs); /// secs, usecs

  EV42Serializer ev42serializer(kafka_buffer_size, "multigrid");
  Producer EventProducer(EFUSettings.KafkaBroker, "C-SPEC_detector");
  ev42serializer.setProducerCallback(std::bind(&Producer::produce2<uint8_t>, &EventProducer, std::placeholders::_1));

//  Multigrid::Sis3153Parser sis3153parser;
//  sis3153parser.buffers.reserve(1000);

  ev42serializer.pulseTime(0);

  uint8_t buffer[eth_buffer_size];
  ssize_t ReadSize {0};
  TSCTimer report_timer;
  // \todo use while
  for (;;) {
    if ((ReadSize = cspecdata.receive(buffer, eth_buffer_size)) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += ReadSize;
      XTRACE(PROCESS, DEB, "Processed UDP packet of size: %d", ReadSize);

      mg_config.builder->parse(Buffer<uint8_t>(buffer, static_cast<size_t>(ReadSize)));

      mystats.discarded_bytes = mg_config.builder->stats_discarded_bytes;
      mystats.triggers = mg_config.builder->stats_trigger_count;
      // \todo more stats

      if (!mg_config.builder->ConvertedData.empty()) {

        mg_config.reduction.ingest(mg_config.builder->ConvertedData);
        mg_config.builder->ConvertedData.clear(); // \todo this should be automatic w ingest
        mg_config.reduction.perform_clustering(false);
        // \todo more stats

        for (auto &event : mg_config.reduction.matcher.matched_events) {
          //XTRACE(PROCESS, DEB, "Analyzing:\n%s", event.debug().c_str());
          auto neutron = mg_config.analyzer.analyze(event);
          if (neutron.good)
            neutrons.push_back(neutron);
        }

        mg_config.reduction.matcher.matched_events.clear();
//        std::sort(neutrons.begin(), neutrons.end(),
//                  [](const Multigrid::NeutronPosition &e1,
//                     const Multigrid::NeutronPosition &e2) {
//                    return e1.time < e2.time;
//                  });

        while (!mg_config.reduction.pulse_times.empty() &&
            !neutrons.empty()) {


          if (!HavePulseTime) {
            ev42serializer.pulseTime(mg_config.reduction.pulse_times.front());
            mg_config.reduction.pulse_times.pop_front();
            XTRACE(PROCESS, DEB, "Initial pulse time: %d",
                   ev42serializer.pulseTime());
            HavePulseTime = true;
          }

          while (!neutrons.empty() &&
              (neutrons.front().time < mg_config.reduction.pulse_times.front())) {
            const auto &neutron = neutrons.front();
//            XTRACE(PROCESS, DEB, "Neutron: %s ", neutron.debug().c_str());
            uint32_t pixel = mg_config.geometry.pixel3D(neutron.x, neutron.y, neutron.z);
            uint32_t time = static_cast<uint32_t>(neutron.time - ev42serializer.pulseTime());
            XTRACE(PROCESS, DEB, "Event: pixel: %d, time: %d ", pixel, time);
            if (pixel == 0) {
              XTRACE(PROCESS, DEB, "Event geom error");
              mystats.geometry_errors++;
            } else if (neutron.time < ev42serializer.pulseTime()) {
              XTRACE(PROCESS, DEB, "Event before pulse");
              mystats.timing_errors++;
            } else if (time > (1.00004 * ShortestPulsePeriod)) {
              XTRACE(PROCESS, DEB, "Event out of pulse range");
              mystats.timing_errors++;
            } else {
//            mystats.readouts_culled += (parsed_readouts - mgEfu->used_readouts);
              XTRACE(PROCESS, DEB, "Event good");
              mystats.events++;
              mystats.tx_bytes += ev42serializer.addEvent(time, pixel);
            }
            neutrons.pop_front();
          }

          if (neutrons.empty())
            break;

          while (!mg_config.reduction.pulse_times.empty() &&
              (mg_config.reduction.pulse_times.front() <
                  neutrons.front().time)) {
            if (ev42serializer.eventCount())
              mystats.tx_bytes += ev42serializer.produce();

            auto PulsePeriod = mg_config.reduction.pulse_times.front()
                - ev42serializer.pulseTime();
            ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);

            ev42serializer.pulseTime(mg_config.reduction.pulse_times.front());
            mg_config.reduction.pulse_times.pop_front();

            XTRACE(PROCESS, DEB, "New pulse time: %d   shortest pulse period: %d",
                   ev42serializer.pulseTime(), ShortestPulsePeriod);
          }

        }
      }
//
//      mystats.sis_discarded_bytes += sis3153parser.parse(Buffer<uint8_t>(buffer, ReadSize));
//
//      for (const auto &b : sis3153parser.buffers) {
//
//        mystats.vmmr_discarded_bytes += vmmr16Parser.parse(b);
//
//        if (vmmr16Parser.converted_data.empty())
//          continue;
//
//        if (dumpfile) {
//          dumpfile->push(vmmr16Parser.converted_data);
//        }
//
//        auto parsed_readouts = vmmr16Parser.converted_data.size();
//
//        if (vmmr16Parser.externalTrigger()) {
//          parsed_readouts--;
//          ev42serializer.pulseTime(RecentPulseTime);
//          if (ev42serializer.eventCount())
//            mystats.tx_bytes += ev42serializer.produce();
//          if (RecentPulseTime) {
//            auto PulsePeriod = vmmr16Parser.time() - RecentPulseTime;
//            ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
//          }
//          RecentPulseTime = vmmr16Parser.time();
//          HavePulseTime = true;
//        }
//
//        mystats.triggers = vmmr16Parser.trigger_count();
//        mystats.readouts += parsed_readouts;
//
//        bool bus_glitch = (vmmr16Parser.converted_data.size() > 40);
//        if (bus_glitch)
//          mystats.bus_glitches++;
//
//        if (mgEfu) {
//          mgEfu->ingest(vmmr16Parser.converted_data);
//
//          if (mgEfu->event_good() && !bus_glitch) {
//            uint32_t pixel = mg_config.geometry.pixel3D(mgEfu->x(), mgEfu->y(), mgEfu->z());
//            uint32_t time = static_cast<uint32_t>(mgEfu->time() - RecentPulseTime);
//
//            XTRACE(PROCESS, DEB, "Event: pixel: %d, time: %d ", pixel, time);
//            if (pixel == 0) {
//              mystats.geometry_errors++;
//            } else if (mgEfu->time() < RecentPulseTime) {
//              mystats.timing_errors++;
//            } else if (time > (1.00004 * ShortestPulsePeriod)) {
//              mystats.timing_errors++;
//            } else if (!HavePulseTime) {
//              mystats.timing_errors++;
//            } else {
//              mystats.readouts_culled += (parsed_readouts - mgEfu->used_readouts);
//              mystats.events++;
//              mystats.tx_bytes += ev42serializer.addEvent(time, pixel);
//            }
//          } else {
//            mystats.readouts_discarded += parsed_readouts;
//            if (!vmmr16Parser.externalTrigger())
//              mystats.bad_triggers++;
//          }
//        }
//      }
//
    }

    // Force periodic flushing
    if (report_timer.timetsc() >= EFUSettings.UpdateIntervalSec * 1000000 * TscMHz) {
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
