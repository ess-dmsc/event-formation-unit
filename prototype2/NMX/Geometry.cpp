/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/Geometry.h>

void Geometry::define_plane(uint16_t planeID,
                                std::initializer_list<std::pair<uint16_t, uint16_t>> chips)
{
  int offset = 0;
  for (auto c : chips)
  {
    set_mapping(c.first, c.second, planeID, offset);
    offset += VMM_TOTAL_CHANNELS;
  }
}

void Geometry::set_mapping(uint16_t fecID, uint16_t vmmID,
                               uint16_t planeID, uint16_t strip_offset)
{
  if (vmmID > 15)
    return;

  if (offsets_.size() <= fecID)
  {
    for (int i = offsets_.size(); i <= fecID; ++i)
    {
      offsets_.resize(i+1);
      offsets_[i] = std::vector<uint16_t>(16, VMM_INVALID);
      planes_.resize(i+1);
      planes_[i] = std::vector<uint16_t>(16, VMM_INVALID);
    }
  }
  offsets_[fecID][vmmID] = strip_offset;
  planes_[fecID][vmmID] = planeID;
}

uint32_t Geometry::get_strip_ID(uint16_t fecID, uint16_t vmmID,
                                    uint32_t channelID) const 
{
  if ((fecID < offsets_.size()) &&
      (vmmID < offsets_.at(fecID).size()) &&
      (offsets_.at(fecID).at(vmmID) != VMM_INVALID))
    return offsets_.at(fecID).at(vmmID) + channelID;
  else
    return VMM_INVALID;
}

uint32_t Geometry::get_plane_ID(uint16_t fecID, uint16_t vmmID) const 
{
  if ((fecID < planes_.size()) &&
      (vmmID < planes_.at(fecID).size()) &&
      (planes_.at(fecID).at(vmmID) != VMM_INVALID))
    return planes_.at(fecID).at(vmmID);
  else
    return VMM_INVALID;
}
