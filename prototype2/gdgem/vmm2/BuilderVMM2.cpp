/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm2/BuilderVMM2.h>
#include <gdgem/clustering/DoroClusterer.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

BuilderVMM2::BuilderVMM2(SRSTime time_intepreter, SRSMappings geometry_interpreter,
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
    readout_file_->open_rw(dump_dir + "gdgem_vmm2_readouts_" + time_str() + ".h5");
  }
}

AbstractBuilder::ResultStats BuilderVMM2::process_buffer(char *buf, size_t size) {
  parser_.receive(buf, size);
  if (!parser_.elems)
    return AbstractBuilder::ResultStats(); // BUG, return error cunters

  readout.fec = 1; /**< \todo not hardcode */
  readout.chip_id = static_cast<uint8_t >(parser_.srshdr.dataid & 0xf);

  geom_errors = 0;

  if (dump_h5_) {
    readout_file_->data.resize(parser_.elems);
  }

  for (unsigned int i = 0; i < parser_.elems; i++) {
    auto &d = parser_.data[i];

    readout.srs_timestamp = parser_.srshdr.time;
    readout.channel = d.chno;
    readout.bcid = d.bcid;
    readout.tdc = d.tdc;
    readout.adc = d.adc;
    readout.over_threshold = (d.overThreshold != 0);

    XTRACE(PROCESS, DEB,
           "srs/vmm timestamp: srs: 0x%08x, bc: 0x%08x, tdc: 0x%08x\n",
           parser_.srshdr.time, d.bcid, d.tdc);
    XTRACE(PROCESS, DEB, "srs/vmm chip: %d, channel: %d\n", readout.chip_id, d.chno);

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
                      readout.chip_id, parser_.srshdr.fc, parser_.srshdr.time, d.chno,
                      d.bcid, d.tdc, d.adc, d.overThreshold);
    }
  }

  if (dump_h5_) {
    readout_file_->write();
  }

  return AbstractBuilder::ResultStats(parser_.elems, parser_.error,
                                      geom_errors);
}
