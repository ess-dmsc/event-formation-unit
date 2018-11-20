/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */

#pragma once

#include <multiblade/caen/Readout.h>

#include <common/clustering/Hit.h>
#include <common/clustering/GapClusterer.h>
#include <common/clustering/GapMatcher.h>

#include <vector>
#include <deque>

static const uint64_t latency{0};
static const uint64_t coordgap{1};
static const uint64_t timegap{125}; // 2us @ 16ns/tick (2000/16)

namespace Multiblade {

class EventBuilder {
public:
  EventBuilder() = default;

  // \todo pass by rvalue?
  void insert(Hit hit);

  void flush();

  void clear();

  std::vector<Hit> p0, p1;

  // \todo parametrize
  GapClusterer c0{timegap, coordgap}, c1{timegap, coordgap};

  // \todo parametrize
  GapMatcher matcher{latency, timegap};
};

}
