/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm2/BuilderVMM2.h>
#include <gdgem/clustering/Clusterer1.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

BuilderVMM2::BuilderVMM2(SRSTime time_intepreter, SRSMappings geometry_interpreter,
                         std::shared_ptr<AbstractClusterer> x, std::shared_ptr<AbstractClusterer> y,
                         std::string dump_dir, bool dump_csv, bool dump_h5)
    : AbstractBuilder(x, y, dump_dir, dump_csv, dump_h5)
    , parser_(1125)
    , time_intepreter_(time_intepreter)
    , geometry_interpreter_(geometry_interpreter)
    , sorter_x(time_intepreter_, geometry_interpreter_, 0, 200, clusterer_x)
    , sorter_y(time_intepreter_, geometry_interpreter_, 0, 200, clusterer_y)
{
  if (dump_csv_) {
    vmmsave->tofile("# fec, chip_id, frame counter, srs timestamp, channel, "
                    "bcid, tdc, adc, overthreshold\n");
  }
  if (dump_h5_) {
    readout_file_ = std::make_shared<ReadoutFile>();
    readout_file_->open_rw(dump_dir + "gdgem_vmm2_readouts_" + time_str() + ".h5");
  }
}

AbstractBuilder::ResultStats BuilderVMM2::process_buffer(char *buf, size_t size,
                                                         NMXHists &hists) {
  parser_.receive(buf, size);
  if (!parser_.elems)
    return AbstractBuilder::ResultStats();

  uint8_t planeID;

  uint16_t fec_id = 1; /**< @todo not hardcode */
  uint16_t chip_id =
      parser_.srshdr.dataid & 0xf; /**< @todo may belong elsewhere */
  uint32_t geom_errors{0};

  if (dump_h5_)
  {
    readout_file_->data.resize(parser_.elems);
  }

  Readout readout;
  for (unsigned int i = 0; i < parser_.elems; i++) {
    auto &d = parser_.data[i];

    readout.fec = fec_id;
    readout.chip_id = chip_id;
    readout.frame_counter = parser_.srshdr.fc;
    readout.srs_timestamp = parser_.srshdr.time;
    readout.channel = d.chno;
    readout.bcid = d.bcid;
    readout.tdc = d.tdc;
    readout.adc = d.adc;
    readout.over_threshold = (d.overThreshold != 0);

    XTRACE(PROCESS, DEB,
           "srs/vmm timestamp: srs: 0x%08x, bc: 0x%08x, tdc: 0x%08x\n",
           parser_.srshdr.time, d.bcid, d.tdc);
    XTRACE(PROCESS, DEB, "srs/vmm chip: %d, channel: %d\n", chip_id, d.chno);
//    XTRACE(PROCESS, DEB,
//           "readout plane_id: %d, strip: %d, time: %f\n",
//           planeID, readout.strip, readout.time);
    /**< @todo flags? */

    planeID = geometry_interpreter_.get_plane(readout.fec, readout.chip_id);

    if (planeID == NMX_INVALID_PLANE_ID) {
      geom_errors++;
      XTRACE(PROCESS, DEB, "Bad SRS mapping --  fec: %d, chip: %d\n", fec_id,
             chip_id);
    } else {
      if (planeID)
        sorter_y.insert(readout);
      else
        sorter_x.insert(readout);

      // TODO: make this optional
      //hists.bin(readout);
      (void) hists;
    }
    if (dump_csv_) {
      vmmsave->tofile("%2d, %2d, %u, %u, %2d, %d, %d, %d, %d\n", fec_id,
                      chip_id, parser_.srshdr.fc, parser_.srshdr.time, d.chno,
                      d.bcid, d.tdc, d.adc, d.overThreshold);
    }
    if (dump_h5_)
    {
      readout_file_->data[i] = readout;
    }
  }

  if (dump_h5_)
  {
    readout_file_->write();
  }

  return AbstractBuilder::ResultStats(parser_.elems, parser_.error,
                                      geom_errors);
}
