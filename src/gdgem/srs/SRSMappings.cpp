/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/srs/SRSMappings.h>
#include <sstream>

namespace Gem {

// This is done only once, when starting EFU
void SRSMappings::define_plane(
    uint8_t planeID,
    std::list<std::pair<uint16_t, uint16_t>> chips) {
  int offset = 0;
  for (auto c : chips) {
    set_mapping(c.first, c.second, planeID, offset);
    offset += MaxChannelsInVMM;
  }
}

// This is done only once, when starting EFU
void SRSMappings::set_mapping(uint16_t fecID, uint16_t vmmID, uint8_t planeID,
                              uint16_t strip_offset) {

  for (uint16_t i = 0; i < MaxChannelsInVMM; ++i) {
    set_mapping(fecID, vmmID, i, planeID, i + strip_offset);
  }
}

void SRSMappings::set_mapping(uint16_t fecID, uint16_t vmmID, uint16_t channel,
                              uint8_t plane, uint16_t coord) {
  if (vmmID >= MaxChipsInFEC)
    return;

  if (channel >= MaxChannelsInVMM)
    return;

  if (mappings_.size() <= fecID)
    mappings_.resize(fecID + 1u);

  auto &FEC = mappings_[fecID];

  if (FEC.size() <= vmmID)
    FEC.resize(vmmID + 1u);

  auto &VMM = FEC[vmmID];

  if (VMM.size() <= channel)
    VMM.resize(channel + 1u);

  VMM[channel] = {plane, coord};
}

/// \todo this seems to validate too much, plane can be
/// valid but channel still invalid?
uint8_t SRSMappings::get_plane(const Readout &readout) const {
  if (readout.fec >= mappings_.size())
    return Hit::InvalidPlane;
  const auto &fec = mappings_[readout.fec];
  if (readout.chip_id >= fec.size())
    return Hit::InvalidPlane;
  const auto &vmm = fec[readout.chip_id];
  printf("vmm.size() %lu\n", vmm.size());
  if (readout.channel >= vmm.size())
    return Hit::InvalidPlane;
  return vmm[readout.channel].plane;
}

uint16_t SRSMappings::get_strip(const Readout &readout) const {
  if (readout.fec >= mappings_.size())
    return Hit::InvalidCoord;
  const auto &fec = mappings_[readout.fec];
  if (readout.chip_id >= fec.size())
    return Hit::InvalidCoord;
  const auto &vmm = fec[readout.chip_id];
  if (readout.channel >= vmm.size())
    return Hit::InvalidCoord;
  return vmm[readout.channel].coordinate;
}

std::string SRSMappings::debug() const {
  std::stringstream ss;
  for (size_t i = 0; i < mappings_.size(); ++i) {
    const auto &fec = mappings_[i];
    for (size_t j = 0; j < fec.size(); ++j) {
      const auto &vmm = fec[j];
      ss << "    (FEC=" << i << ",VMM=" << j << "):";
      for (size_t k = 0; k < vmm.size(); ++k) {
        if ((k % 16) == 0)
          ss << "\n      ";
        ss << k << ":";
        const auto &m = vmm[k];
        if ((m.plane == Hit::InvalidPlane) || (m.coordinate == Hit::InvalidCoord))
          ss << "x ";
        else
          ss << static_cast<int32_t>(m.plane)
             << "/" << static_cast<int32_t>(m.coordinate) << " ";
      }
      ss << "\n";
    }
  }
  return ss.str();
}

}
