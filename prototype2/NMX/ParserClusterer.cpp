/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/ParserClusterer.h>
#include <string.h>

void ParserClusterer::parse(char *buf, size_t size) {
  vmm_nugget packet;
  size_t psize = sizeof(packet);

  size_t limit = std::min(size / psize, size_t(9000 / psize));
  size_t byteidx = 0;
  for (; byteidx < limit; ++byteidx) {
    memcpy(&packet, buf, sizeof(packet));
    buf += psize;

    backlog_.insert(std::pair<uint64_t, vmm_nugget>(packet.time, packet));
  }
}

bool ParserClusterer::event_ready() {
  return (!backlog_.empty() &&
          ((backlog_.rbegin()->first - backlog_.begin()->first) > 30));
}

EventNMX ParserClusterer::get() {
  if (!event_ready())
    return EventNMX();
  EventNMX ret;
  auto latest = backlog_.begin()->first + 30;
  while (backlog_.begin()->first <= latest) {
    ret.push(backlog_.begin()->second);
    backlog_.erase(backlog_.begin());
  }
  return ret;
}
