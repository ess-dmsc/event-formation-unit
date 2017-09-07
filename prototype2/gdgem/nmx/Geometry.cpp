#include <gdgem/nmx/Geometry.h>

void Geometry::add_dimension(uint16_t size)
{
  if (!size)
    return; //throw instead?
  if (coefs_.empty())
    coefs_.push_back(1);
  coefs_.push_back(size * coefs_.back());
  limits_.push_back(size);
  maxid_ = coefs_.back();
}

uint32_t Geometry::to_pixid(const std::vector<uint16_t>& coords) const
{
  if (coords.size() != limits_.size())
    return 0;
  uint32_t ret {1};
  for (uint16_t i=0; i < coords.size(); ++i)
  {
    const auto& c = coords[i];
    if (limits_[i] <= c)
      return 0;
    ret += coefs_[i] * c;
  }
  return ret;
}

bool Geometry::from_pixid(uint32_t pixid, std::vector<uint16_t>& coords) const
{
  if (!pixid ||
      limits_.empty() ||
      (pixid > maxid_) ||
      (coords.size() != limits_.size()))
    return false;

  pixid--;
  for (int32_t i=limits_.size()-1; i >= 0; --i)
  {
    const auto& c = coefs_[i];
    coords[i] = pixid / c;
    pixid = pixid % c;
  }
  return (pixid == 0);
}

