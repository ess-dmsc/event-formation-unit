/** Copyright (C) 2016-2018 European Spallation Source ERIC */

#include <gdgem/srs/BuilderVMM3.h>
#include <gdgem/NMXStats.h>
#include <common/reduction/clustering/GapClusterer.h>
#include <common/TimeString.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
// #undef TRC_MASK
// #define TRC_MASK TRC_M_NONE

namespace Gem {

BuilderVMM3::BuilderVMM3(SRSTime time_intepreter,
                         SRSMappings  digital_geometry,
                         uint16_t adc_threshold,
                         std::string dump_dir,
                         unsigned int pmin,
                         unsigned int pmax,
                         unsigned int pwidth,
                         std::shared_ptr<CalibrationFile> calfile, NMXStats & stats, bool enable_data_processing)
                         : time_intepreter_(time_intepreter)
                         , digital_geometry_(digital_geometry)
                         , adc_threshold_ (adc_threshold)
                         , PMin(pmin)
                         , PMax(pmax)
                         , PWidth(pwidth)
                         , stats_(stats),parser_(1500, stats,time_intepreter), data_processing_(enable_data_processing)
                          {
  assert(calfile != nullptr);
  calfile_ = calfile;

  if (!dump_dir.empty()) {
    readout_file_ = ReadoutFile::create(dump_dir + "gdgem_readouts_" + timeString(), 1000);
  }
}


void BuilderVMM3::process_buffer(char *buf, size_t size) {
  int hits = parser_.receive(buf, size);
  if (!hits) {
    XTRACE(PROCESS, DEB, "No readouts after parse");
    return;
  }
  XTRACE(PROCESS, DEB, "Readouts after parse: %d", hits);

  // reserve some space up front to avoid allocator noise
  hit_buffer_x.reserve (1000);
  hit_buffer_y.reserve (1000);

  //field fec id starts at 1
  readout.fec = parser_.pd.fecId;
  for (int i = 0; i < hits; i++) {
    auto &d = parser_.data[i];
    if (d.hasDataMarker) {
      // \todo should these be functions of SRSTime?
      readout.srs_timestamp = (
          static_cast<uint64_t>(d.fecTimeStamp) * SRSTime::internal_SRS_clock_period_ns
              + static_cast<uint64_t>(d.triggerOffset) * time_intepreter_.trigger_period_ns()
      );

      readout.chip_id = d.vmmid;
      readout.channel = d.chno;
      readout.bcid = d.bcid;
      readout.tdc = d.tdc;
      readout.over_threshold = (d.overThreshold != 0);

      auto calib = calfile_->getCalibration(readout.fec, readout.chip_id, readout.channel);
      // \todo does this really need to be a floating point value?
      readout.chiptime = static_cast<float>(time_intepreter_.chip_time_ns(d.bcid, d.tdc,
          calib.time_offset, calib.time_slope));
      // \todo what if chiptime is negative?
      readout.adc = (d.adc- calib.adc_offset)*calib.adc_slope;

      XTRACE(PROCESS, DEB,
             "srs/vmm timestamp: srs: 0x%08x, bc: 0x%08x, tdc: 0x%08x",
             readout.srs_timestamp, d.bcid, d.tdc);
      XTRACE(PROCESS, DEB, "srs/vmm chip: %d, channel: %d",
             readout.chip_id, d.chno);


      if (readout_file_) {
        readout_file_->push(readout);
      }

      if(data_processing_) {
        hit.plane = digital_geometry_.get_plane(readout);
        hit.coordinate = digital_geometry_.get_strip(readout);
        hit.weight = readout.adc;
        hit.time = readout.srs_timestamp;
        if (readout.chiptime >= 0)
          hit.time += static_cast<uint64_t>(readout.chiptime);
        else
          hit.time -= static_cast<uint64_t>(-readout.chiptime);


        if ((hit.plane != 0) && (hit.plane != 1)) {
          stats_.HitsBadPlane++;
          XTRACE(PROCESS, DEB, "Bad SRS mapping (plane) -- fec=%d, chip=%d",
                readout.fec, readout.chip_id);
          continue;
        }

        if (hit.coordinate == Hit::InvalidCoord) {
          stats_.HitsBadGeometry++;
          XTRACE(PROCESS, DEB, "Bad SRS mapping (coordinate) -- fec=%d, chip=%d",
              readout.fec, readout.chip_id);
          continue;
        }

        if (hit.weight == 0 || (!readout.over_threshold && hit.weight < adc_threshold_)) {
          stats_.HitsBadAdc++;
          XTRACE(PROCESS, DEB, "ADC=0 or ADC below threshold  adc=%d", hit.weight);
          continue;
        }

        if (hit.plane == 1) {
          hit_buffer_y.emplace_back(hit);
        }

        if (hit.plane == 0) {
          auto c = hit.coordinate;
          if ( c >= std::max(0, (int)(PMin - PWidth)) and (c <= std::min(1279, (int)(PMax + PWidth)))) {
            hit_buffer_x.emplace_back(hit);
          } else {
            stats_.HitsOutsideRegion++;
          }
        }

      } else {
        LOG(PROCESS, Sev::Warning, "No data marker in hit (increment counter?)");
      }
    }
  }
}

}
