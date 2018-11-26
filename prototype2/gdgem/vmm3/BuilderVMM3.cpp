/** Copyright (C) 2016-2018 European Spallation Source ERIC */

#include <gdgem/vmm3/BuilderVMM3.h>
#include <gdgem/clustering/DoroClusterer.h>
#include <common/TimeString.h>

#include <common/Trace.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Gem {

BuilderVMM3::BuilderVMM3(SRSTime time_intepreter,
						 SRSMappings geometry_interpreter, std::shared_ptr<AbstractClusterer> x,
						 std::shared_ptr<AbstractClusterer> y, uint16_t adc_threshold_x,
						 double max_time_gap_x, uint16_t adc_threshold_y, double max_time_gap_y,
						 std::string dump_dir, std::shared_ptr<CalibrationFile> calfile) :
		AbstractBuilder(x, y), parser_(1500), time_intepreter_(
		time_intepreter), geometry_interpreter_(geometry_interpreter), sorter_x(
		time_intepreter_, geometry_interpreter_, adc_threshold_x,
		max_time_gap_x), sorter_y(time_intepreter_,
								  geometry_interpreter_, adc_threshold_y, max_time_gap_y) {

	assert(calfile != nullptr);
	calfile_ = calfile;
	clusterer_x = x;
	clusterer_y = y;
	sorter_x.clusterer = clusterer_x;
	sorter_y.clusterer = clusterer_y;

	if (!dump_dir.empty()) {
		readout_file_ = ReadoutFile::create(dump_dir + "gdgem_readouts_" + timeString(), 1000);
	}
}

AbstractBuilder::ResultStats BuilderVMM3::process_buffer(char *buf, size_t size) {
	geom_errors = 0;
	parser_.receive(buf, size);
	if (!parser_.stats.hits) {
		XTRACE(PROCESS, DEB, "NO HITS after parse");
		auto &stats = parser_.stats;
		return AbstractBuilder::ResultStats(stats.hits, stats.errors,
											geom_errors, stats.lostFrames, stats.badFrames,
											stats.goodFrames);
	}
	XTRACE(PROCESS, DEB, "HITS after parse: %d", parser_.stats.hits);


//	uint32_t udp_timestamp_ns = parser_.srsHeader.udpTimeStamp
//			* time_intepreter_.internal_clock_period_ns();

	//field fec id starts at 1
	readout.fec = parser_.parserData.fecId;
	for (unsigned int i = 0; i < parser_.stats.hits; i++) {
		auto &d = parser_.data[i];
		if (d.hasDataMarker) {
			readout.srs_timestamp = d.fecTimeStamp
					* time_intepreter_.internal_clock_period_ns()
					+ d.triggerOffset * time_intepreter_.trigger_period_ns();

			readout.chip_id = d.vmmid;
			readout.channel = d.chno;
			readout.bcid = d.bcid;
			readout.tdc = d.tdc;
			readout.adc = d.adc;
			readout.over_threshold = (d.overThreshold != 0);
			auto calib = calfile_->getCalibration(readout.fec, readout.chip_id, readout.channel);
			readout.chiptime = time_intepreter_.chip_time_ns(d.bcid, d.tdc, calib.offset, calib.slope);

			XTRACE(PROCESS, DEB,
				   "srs/vmm timestamp: srs: 0x%08x, bc: 0x%08x, tdc: 0x%08x",
				   readout.srs_timestamp, d.bcid, d.tdc);
			XTRACE(PROCESS, DEB, "srs/vmm chip: %d, channel: %d",
				   readout.chip_id, d.chno);

			plane = geometry_interpreter_.get_plane(readout);
			if (plane != NMX_INVALID_PLANE_ID) {
				if (plane) {
					sorter_y.insert(readout);
				} else {
					sorter_x.insert(readout);

				}
			} else {
				geom_errors++;

				XTRACE(PROCESS, DEB, "Bad SRS mapping --  fec: %d, chip: %d",
					   readout.fec, readout.chip_id);
			}

			if (readout_file_) {
				readout_file_->push(readout);
			}

		} else {
			XTRACE(PROCESS, DEB, "No data marker in hit (increment counter?)");
		}
	}

	auto &stats = parser_.stats;
	return AbstractBuilder::ResultStats(stats.hits, stats.errors, geom_errors,
										stats.lostFrames, stats.badFrames, stats.goodFrames);
}

}
