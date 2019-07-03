#=============================================================================
# Common functionality for all detector plugins
#=============================================================================

set(clustering_obj_SRC
  AbstractMatcher.cpp
  Cluster.cpp
  EndMatcher.cpp
  Event.cpp
  GapClusterer.cpp
  GapMatcher.cpp
  Hit.cpp
  OverlapMatcher.cpp
)

set(clustering_obj_INC
  AbstractClusterer.h
  AbstractMatcher.h
  Cluster.h
  EndMatcher.h
  Event.h
  GapClusterer.h
  GapMatcher.h
  Hit.h
  HitVector.h
  OverlapMatcher.h
)

add_library(ClusteringObj OBJECT ${clustering_obj_SRC} ${clustering_obj_INC})
enable_coverage_flags(ClusteringObj)

add_subdirectory(test)

