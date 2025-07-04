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

add_library(MonitorLib OBJECT
  ${monitor_obj_SRC}
  ${monitor_obj_INC}
  )

target_link_libraries(MonitorLib 
  PRIVATE
    ${EFU_COMMON_LIBS}
)

enable_coverage_flags(MonitorLib)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
