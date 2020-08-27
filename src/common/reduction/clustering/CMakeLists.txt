#=============================================================================
# Classes for hit clustering
#=============================================================================

include_directories(.)
include_directories(${ESS_MODULE_DIR})

set(clustering_obj_SRC
  AbstractClusterer.cpp
  GapClusterer.cpp
  GapClusterer2D.cpp
)

set(clustering_obj_INC
  AbstractClusterer.h
  GapClusterer.h
  GapClusterer2D.h
)

add_library(ClusteringObj OBJECT
  ${clustering_obj_SRC}
  ${clustering_obj_INC}
  )

enable_coverage_flags(ClusteringObj)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
