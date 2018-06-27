/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <gdgem/clustering/DoroClusterer.h>
#include <gdgem/vmm3/BuilderVMM3.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

BuilderVMM3::BuilderVMM3(SRSTime time_intepreter, SRSMappings geometry_interpreter,
                         std::shared_ptr<AbstractClusterer> x,
                         std::shared_ptr<AbstractClusterer> y,
                         uint16_t adc_threshold_x, double max_time_gap_x,
                         uint16_t adc_threshold_y, double max_time_gap_y,
                         std::string dump_dir, bool dump_csv, bool dump_h5)
    : AbstractBuilder(x, y, dump_dir, dump_csv, dump_h5), parser_(1125), time_intepreter_(time_intepreter),
      geometry_interpreter_(geometry_interpreter),
      sorter_x(time_intepreter_, geometry_interpreter_, adc_threshold_x, max_time_gap_x),
      sorter_y(time_intepreter_, geometry_interpreter_, adc_threshold_y, max_time_gap_y) {
  clusterer_x = x;
  clusterer_y = y;
  sorter_x.clusterer = clusterer_x;
  sorter_y.clusterer = clusterer_y;
  if (dump_csv_) {
    vmmsave->tofile("# fec, chip_id, frame counter, srs timestamp, channel, "
                    "bcid, tdc, adc, overthreshold\n");
  }
  if (dump_h5_) {
    readout_file_ = std::make_shared<ReadoutFile>();
    readout_file_->open_rw(dump_dir + "gdgem_vmm3_readouts_" + time_str() + ".h5");
  }
}

AbstractBuilder::ResultStats BuilderVMM3::process_buffer(char *buf, size_t size) {
  parser_.receive(buf, size);
  if (!parser_.elems)
    return AbstractBuilder::ResultStats();

  geom_errors = 0;

  if (dump_h5_) {
    readout_file_->data.resize(parser_.elems);
  }



  for (unsigned int i = 0; i < parser_.elems; i++) {
    /// TODO two next lines could be moved out of the loop
    readout.fec = parser_.srshdr.fec;
    readout.srs_timestamp = parser_.srshdr.txtime;

    auto &d = parser_.data[i];
    readout.chip_id = d.vmmid;
    readout.channel = d.chno;
    readout.bcid = d.bcid;
    readout.tdc = d.tdc;
    readout.adc = d.adc;
    readout.over_threshold = (d.overThreshold != 0);

    XTRACE(PROCESS, DEB, "fec: %d, vmmid: %d, channel: %d\n", readout.fec, readout.chip_id, readout.channel);
    XTRACE(PROCESS, DEB,
           "srs/vmm timestamp: srs: 0x%08x, bc: 0x%d, tdc: 0x%d, adc: %d\n",
           readout.srs_timestamp, readout.bcid, readout.tdc, readout.adc);


    plane = geometry_interpreter_.get_plane(readout);

    if (plane != NMX_INVALID_PLANE_ID) {
      if (plane)
        sorter_y.insert(readout);
      else
        sorter_x.insert(readout);
    } else {
      geom_errors++;
      XTRACE(PROCESS, DEB, "Bad SRS mapping --  fec: %d, chip: %d\n", readout.fec,
             readout.chip_id);
    }

    if (dump_h5_) {
      readout_file_->data[i] = readout;
    }

    if (dump_csv_) {
      vmmsave->tofile("%2d, %2d, %u, %u, %2d, %d, %d, %d, %d\n", readout.fec,
                      readout.chip_id, parser_.srshdr.fc, parser_.srshdr.txtime, d.chno,
                      d.bcid, d.tdc, d.adc, d.overThreshold);
    }
  }

  if (dump_h5_) {
    readout_file_->write();
  }

  return AbstractBuilder::ResultStats(parser_.elems, parser_.error,
                                      geom_errors);
}
