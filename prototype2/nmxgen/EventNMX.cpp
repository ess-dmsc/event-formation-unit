/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <nmxgen/EventNMX.h>

void PlaneNMX::push(const EntryNMX& e)
{
  if (!e.adc)
    return;
  if (entries.empty())
    time_start = time_end = e.time;
  entries.push_back(e);
  integral += e.adc;
  time_start = std::min(time_start, e.time);
  time_end = std::max(time_start, e.time);
}
  
void PlaneNMX::analyze(bool weighted, uint16_t max_timebins, uint16_t max_timedif)
{
  double center_sum {0};
  double center_count {0};
  int16_t lspan_min = std::numeric_limits<int16_t>::max();
  int16_t lspan_max = std::numeric_limits<int16_t>::min();
  int16_t uspan_min = std::numeric_limits<int16_t>::max();
  int16_t uspan_max = std::numeric_limits<int16_t>::min();
  uint64_t earliest = time_end - max_timedif;
  std::set<uint64_t> timebins;
  for (auto it = entries.rbegin(); it != entries.rend(); ++it)
  {
    auto e = *it;
    if (e.time == time_end)
    {
      if (weighted)
      {
        center_sum += e.strip * e.adc;
        center_count += e.adc;
      }
      else
      {
        center_sum += e.strip;
        center_count++;
      }
      lspan_min = std::min(lspan_min, static_cast<int16_t>(e.strip));
      lspan_max = std::max(lspan_max, static_cast<int16_t>(e.strip));
    }
    if ((timebins.size() < max_timebins) && (e.time >= earliest))
    {
      timebins.insert(e.time);
      uspan_min = std::min(uspan_min, static_cast<int16_t>(e.strip));
      uspan_max = std::max(uspan_max, static_cast<int16_t>(e.strip));        
    }
    else
      break;
  }
  center = center_sum / center_count;
  uncert_lower = lspan_max - lspan_min;
  uncert_upper = uspan_max - uspan_min;
} 


void EventNMX::push(const EntryNMX& e)
{
  if (e.plane_id)
    y.push(e);
  else
    x.push(e);
}
  
void EventNMX::analyze(bool weighted, int16_t max_timebins, int16_t max_timedif)
{
  if (x.entries.size())
    x.analyze(weighted, max_timebins, max_timedif);
  if (y.entries.size())
    y.analyze(weighted, max_timebins, max_timedif);
  good = x.entries.size() && y.entries.size();
}
  
