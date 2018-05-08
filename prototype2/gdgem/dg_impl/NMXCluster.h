#pragma once

#include <vector>

#include <gdgem/nmx/Eventlet.h>
#include <gdgem/nmx/EventNMX.h>

struct ClusterNMX {
//  size_t size;
//  uint64_t adc;
//  double position;
//  double time;
  bool clusterXAndY {false};
//  double maxDeltaTime;
//  double maxDeltaStrip;
//  double deltaSpan;
  PlaneNMX plane;
};

using HitContainer = std::vector<Eventlet>;
using ClusterVector = std::vector<ClusterNMX>;
