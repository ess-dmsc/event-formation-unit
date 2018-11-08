/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */

#pragma once

#include <multiblade/caen/Readout.h>

#include <common/clustering/Hit.h>
#include <common/clustering/GapClusterer.h>
#include <common/clustering/OverlapMatcher.h>

#include <vector>
#include <deque>

namespace Multiblade {

class EventBuilder2 {
public:
  EventBuilder2() = default;

  // \todo pass by rvalue?
  void insert(Hit hit);

  void flush();

  void clear();

  std::vector<Hit> p0, p1;

  GapClusterer c0{300,0}, c1{300,0};

  OverlapMatcher matcher{1600};
};


}
