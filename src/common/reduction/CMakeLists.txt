#=============================================================================
# Classes for event reduction pipeline
#=============================================================================

add_subdirectory(clustering)
add_subdirectory(matching)
add_subdirectory(analysis)

set(reduction_obj_SRC
  Hit.cpp
  HitVector.cpp
  Cluster.cpp
  Event.cpp
  EventBuilder2D.cpp
  ReducedEvent.cpp
  NeutronEvent.cpp
  ChronoMerger.cpp
)

set(reduction_obj_INC
  Hit.h
  HitVector.h
  Cluster.h
  Event.h
  EventBuilder2D.h
  ReducedEvent.h
  NeutronEvent.h
  ChronoMerger.h
)

add_library(ReductionLib OBJECT
  ${reduction_obj_SRC}
  ${reduction_obj_INC}
  )

enable_coverage_flags(ReductionLib)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
