#=============================================================================
# Classes for hit and histogram serialization for monitor streams
#=============================================================================

set(monitor_obj_SRC
  Histogram.cpp
  HistogramSerializer.cpp
  DynamicHist.cpp
  HitSerializer.cpp
)

set(monitor_obj_INC
  Histogram.h
  HistogramSerializer.h
  DynamicHist.h
  HitSerializer.h
)

add_library(MonitorObj OBJECT
  ${monitor_obj_SRC}
  ${monitor_obj_INC}
  )

enable_coverage_flags(MonitorObj)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
