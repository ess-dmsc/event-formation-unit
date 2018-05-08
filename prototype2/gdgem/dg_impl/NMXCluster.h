#pragma once

#include <vector>

#include <gdgem/nmx/Eventlet.h>

struct ClusterNMX {
  int size;
  int adc;
  double position;
  double time;
  bool clusterXAndY;
  double maxDeltaTime;
  double maxDeltaStrip;
  double deltaSpan;
};

using HitContainer = std::vector<Eventlet>;
using ClusterVector = std::vector<ClusterNMX>;
