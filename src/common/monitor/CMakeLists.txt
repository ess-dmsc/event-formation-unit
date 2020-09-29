#=============================================================================
# Classes for hit and histogram serialization for monitor streams
#=============================================================================

set(monitor_obj_SRC
  Histogram.cpp
  HistogramSerializer.cpp
  DynamicHist.cpp
  HitSerializer.cpp
  Monitor.cpp
)

set(monitor_obj_INC
  Histogram.h
  HistogramSerializer.h
  DynamicHist.h
  HitSerializer.h
  Monitor.h
)

add_library(MonitorLib OBJECT
  ${monitor_obj_SRC}
  ${monitor_obj_INC}
  )

enable_coverage_flags(MonitorLib)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
