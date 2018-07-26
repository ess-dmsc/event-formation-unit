/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simulator/Generator for Gd-GEM
///
//===----------------------------------------------------------------------===//

#pragma once

#include <Event.h>
#include <Geometry.h>
#include <queue>

class Simulator {
public:
  Geometry geom{600, 600, 1280, 1280};
  double time{0.0};

  std::priority_queue<Event*, std::vector<Event *, std::allocator<Event*> >,
                          Event::eventComparator> eventQueue;

  Simulator() {};

  double now() {
    return time;
  }

  void addEvent(Event * e) {
    eventQueue.push(e);
  }

  void run() {
    while ( !eventQueue.empty() ) {
      auto e = eventQueue.top();
      eventQueue.pop();
      time = e->time;
      e->execute(this);
      delete e;
    }
  }
};
