/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include<nmxgen/ParserClusterer.h>
#include<nmxgen/ReaderVMM.h>

void ParserClusterer::parse (char* buf, size_t size)
{
  PacketVMM packet;

  size_t limit = std::min(size / 12, size_t(9000/12));
  size_t byteidx = 0;
  for (; byteidx < limit; ++byteidx)
  {
    memcpy(&packet, buf, sizeof(packet));
    buf += 12;

    EntryNMX entry;
    entry.time = (static_cast<uint64_t>(packet.time_offset) << 32) + packet.timebin;
    entry.plane_id = packet.plane_id;
    entry.strip = packet.strip;
    entry.adc = packet.adc;
    backlog_.insert(std::pair<uint64_t, EntryNMX>(entry.time, entry));
  }
}

bool ParserClusterer::event_ready()
{
  return (!backlog_.empty() &&
          ((backlog_.rbegin()->first - backlog_.begin()->first) > 30));
}

EventNMX ParserClusterer::get()
{
  if (!event_ready())
    return EventNMX();
  EventNMX ret;
  auto latest = backlog_.begin()->first + 30;
  while (backlog_.begin()->first <= latest)
  {
    ret.push(backlog_.begin()->second);
    backlog_.erase(backlog_.begin());
  }
  return ret;
}
