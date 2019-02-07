/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mesytec/BuilderMesytec.h>
#include <common/TimeString.h>

#include <common/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

BuilderMesytec::BuilderMesytec(const SequoiaGeometry &geometry, bool spoof_time,
                               std::string dump_dir)
    : BuilderReadouts(geometry) {
  vmmr16Parser_.spoof_high_time(spoof_time);
  if (!dump_dir.empty()) {
    dumpfile_ = ReadoutFile::create(dump_dir + "mgmesytec_readouts_" + timeString(), 100);
  }

}

std::string BuilderMesytec::debug() const {
  std::stringstream ss;
  ss << "  =====================================================\n";
  ss << "  ========           Mesytec Builder           ========\n";
  ss << "  =====================================================\n";

  ss << "  Spoof high time (vmmr16):"
     << (vmmr16Parser_.spoof_high_time() ? "YES" : "no") << "\n";

  ss << BuilderReadouts::debug();

  return ss.str();
}

void BuilderMesytec::parse(Buffer<uint8_t> buffer) {

  stats_discarded_bytes += sis3153parser_.parse(Buffer<uint8_t>(buffer));

  for (const auto &b : sis3153parser_.buffers) {

    stats_discarded_bytes += vmmr16Parser_.parse(b);

    stats_trigger_count = vmmr16Parser_.trigger_count();

    if (vmmr16Parser_.converted_data.empty())
      continue;

    // \todo make optional and parametrize threshold
    if (vmmr16Parser_.converted_data.size() > 40) {
      stats_bus_glitch_rejects += vmmr16Parser_.converted_data.size();
      continue;
    }

    if (dumpfile_) {
      dumpfile_->push(vmmr16Parser_.converted_data);
    }

    build(vmmr16Parser_.converted_data);
  }
}

}
