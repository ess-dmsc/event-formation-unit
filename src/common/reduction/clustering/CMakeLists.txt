#=============================================================================
# Classes for hit clustering
#=============================================================================

include_directories(.)
include_directories(${ESS_MODULE_DIR})

set(clustering_obj_SRC
AbstractClusterer.cpp
Abstract2DClusterer.cpp
Hierarchical2DClusterer.cpp
  GapClusterer.cpp
  GapClusterer2D.cpp
)

set(clustering_obj_INC
AbstractClusterer.h
Abstract2DClusterer.h
Hierarchical2DClusterer.h
  GapClusterer.h
  GapClusterer2D.h
)

add_library(ClusteringLib OBJECT
  ${clustering_obj_SRC}
  ${clustering_obj_INC}
  )

target_link_libraries(ClusteringLib
  PRIVATE
    ${EFU_COMMON_LIBS}
)

enable_coverage_flags(ClusteringLib)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
