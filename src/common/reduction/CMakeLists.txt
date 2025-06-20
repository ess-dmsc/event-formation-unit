#=============================================================================
# Classes for event reduction pipeline
#=============================================================================

add_subdirectory(clustering)
add_subdirectory(matching)
add_subdirectory(analysis)
add_subdirectory(multigrid)

set(reduction_obj_SRC
  Hit.cpp
  HitVector.cpp
  Hit2D.cpp
  Hit2DVector.cpp
  Cluster.cpp
  Cluster2D.cpp
  Event.cpp
  EventBuilder2D.cpp
  ReducedEvent.cpp
  NeutronEvent.cpp
  ChronoMerger.cpp
)

set(reduction_obj_INC
  Hit.h
  HitVector.h
  Hit2D.h
  Hit2DVector.h
  Cluster.h
  Cluster2D.h
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

target_link_libraries(ReductionLib
  PRIVATE
    ${EFU_COMMON_LIBS}
)

enable_coverage_flags(ReductionLib)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
