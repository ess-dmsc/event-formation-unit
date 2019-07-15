#=============================================================================
# Classes for hit clustering
#=============================================================================

set(clustering_obj_SRC
  AbstractClusterer.cpp
  GapClusterer.cpp
)

set(clustering_obj_INC
  AbstractClusterer.h
  GapClusterer.h
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

