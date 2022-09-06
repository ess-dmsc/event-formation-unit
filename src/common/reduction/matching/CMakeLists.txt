#=============================================================================
# Classes for cluster matching across planes
#=============================================================================

set(matching_obj_SRC
  AbstractMatcher.cpp
  OverlapMatcher.cpp
  GapMatcher.cpp
  EndMatcher.cpp
  CenterMatcher.cpp
  MultiHitMatcher2D.cpp
)

set(matching_obj_INC
  AbstractMatcher.h
  OverlapMatcher.h
  GapMatcher.h
  EndMatcher.h
  CenterMatcher.h
  MultiHitMatcher2D.h
)

add_library(MatchingLib OBJECT
  ${matching_obj_SRC}
  ${matching_obj_INC}
  )

enable_coverage_flags(MatchingLib)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)

