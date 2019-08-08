/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/EventAnalyzer.h>
#include <cmath>
#include <set>
#include <sstream>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
#undef TRC_MASK
#define TRC_MASK 0

namespace Gem {

uint32_t ResultsPlane::coord_rounded() const {
  return static_cast<uint32_t>(std::round(coord_));
}

std::string ResultsPlane::debug() const {
  return fmt::format("{}(lu={},uu={})", coord_, uncert_lower_, uncert_upper_);
}

std::string Results::debug() const {
  return fmt::format("x={}, y={}, t={}", x_.debug(), y_.debug(), time_);
}


EventAnalyzer::EventAnalyzer(std::string time_algorithm)
: time_algorithm_(time_algorithm)
{}

ResultsPlane EventAnalyzer::analyze(Cluster& cluster) const {
  ResultsPlane ret;

  if (cluster.hits.empty()) {
    return ret;
  }

  ret.coord_ = cluster.coord_utpc(false);   
  if(time_algorithm_ == "center-of-mass") {
    ret.coord_ = cluster.coord_center();
    ret.time_ = cluster.time_center(); 
  }
  else if(time_algorithm_ == "charge2") {
    ret.coord_ = cluster.coord_center2();   
    ret.time_ = cluster.time_center2(); 
  }  
  else if(time_algorithm_ == "utpc") {
    ret.coord_ = cluster.coord_utpc(false);   
    ret.time_ = cluster.time_utpc(false); 
  }
  // "utpc_weighted"
  else {
    ret.coord_ = cluster.coord_utpc(true); 
    ret.time_ = cluster.time_utpc(true); 
  }
  
  ret.uncert_lower_ = cluster.lspan_max() - cluster.lspan_min() + int16_t(1);
  ret.uncert_upper_ = 0;
  return ret;
}

Results EventAnalyzer::analyze(Event& event) const {
  Results ret;
  ret.x_ = analyze(event.c1);
  ret.y_ = analyze(event.c2);
  ret.good_ = std::isfinite(ret.x_.coord_) && std::isfinite(ret.y_.coord_);
  ret.time_ = std::min(ret.x_.time_, ret.y_.time_);
  return ret;
}

bool EventAnalyzer::meets_lower_criterion(const ResultsPlane& x, const ResultsPlane& y,
                                  int16_t max_lu) {
  return (x.uncert_lower_ < max_lu) && (y.uncert_lower_ < max_lu);
}



}
