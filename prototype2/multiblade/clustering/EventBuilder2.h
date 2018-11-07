/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */

#pragma once

#include <common/clustering/Hit.h>
#include <common/Trace.h>
#include <multiblade/caen/Readout.h>

#include <vector>
#include <deque>

namespace Multiblade {

class HitQueue {
public:
  HitQueue() = default;

  // \todo pass by rvalue?
  void insert(Readout hit);

  std::deque<Readout> readouts;

  uint16_t id{0};
};

class DigitizerQueue {
public:
  DigitizerQueue();

  // \todo pass by rvalue?
  void insert(Readout hit);

  std::vector<HitQueue> groups;

  uint16_t id{0};
};

}
