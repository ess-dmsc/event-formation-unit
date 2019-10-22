/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/reduction/analysis/EventAnalyzer.h>
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

EventAnalyzer::EventAnalyzer(std::string time_algorithm)
: time_algorithm_(time_algorithm)
{}

ReducedHit EventAnalyzer::analyze(Cluster& cluster) const {
  ReducedHit ret;

  if (cluster.hits.empty()) {
    return ret;
  }

  if(time_algorithm_ == "center-of-mass") {
    ret.center = cluster.coord_center();
    ret.time = cluster.time_center(); 
  }
  else if(time_algorithm_ == "charge2") {
    ret.center = cluster.coord_center2();   
    ret.time = cluster.time_center2(); 
  }  
  else {
    if(time_algorithm_ == "utpc") {
      ret.center = cluster.coord_utpc(false);  
    } else {
      ret.center = cluster.coord_utpc(true);  
    } 
    ret.time = cluster.time_end(); 
  }
  
  return ret;
}

ReducedEvent EventAnalyzer::analyze(Event& event) const {
  ReducedEvent ret;
  ret.x = analyze(event.ClusterA);
  ret.y = analyze(event.ClusterB);
  ret.good = std::isfinite(ret.x.center) && std::isfinite(ret.y.center);
  ret.time = std::min(ret.x.time, ret.y.time);
  return ret;
}

std::string EventAnalyzer::debug(const std::string& prepend) const {
  std::stringstream ss;
  ss << "Event analysis\n";
  ss << prepend << fmt::format("  time_algorithm = {}\n", time_algorithm_ );
  return ss.str();
}