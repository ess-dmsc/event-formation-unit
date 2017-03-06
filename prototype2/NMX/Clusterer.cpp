/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/Clusterer.h>
#include <common/Trace.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

Clusterer::Clusterer(uint64_t min_time_span)
  : min_time_span_(min_time_span)
{}

void Clusterer::insert(const Eventlet& eventlet) {
  backlog_.insert(std::pair<uint64_t, Eventlet>(eventlet.time, eventlet));    
}


bool Clusterer::event_ready() const {
  return (!backlog_.empty() &&
          ((backlog_.rbegin()->first - backlog_.begin()->first) > min_time_span_));
}

EventNMX Clusterer::get_event() {
  if (!event_ready())
    return EventNMX();
  EventNMX ret;
  auto latest = backlog_.begin()->first + min_time_span_;
  while (backlog_.begin()->first <= latest) {
    ret.insert_eventlet(backlog_.begin()->second);
    backlog_.erase(backlog_.begin());
  }
  return ret;
}
