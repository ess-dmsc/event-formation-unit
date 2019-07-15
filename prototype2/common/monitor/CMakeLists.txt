#=============================================================================
# Classes for hit and histogram serialization for monitor streams
#=============================================================================

set(monitor_obj_SRC
  Hists.cpp
  HistSerializer.cpp
  DynamicHist.cpp
  ReadoutSerializer.cpp
)

set(monitor_obj_INC
  Hists.h
  HistSerializer.h
  DynamicHist.h
  ReadoutSerializer.h
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

