#=============================================================================
# Classes for cluster matching across planes
#=============================================================================

set(matching_obj_SRC
  AbstractMatcher.cpp
  OverlapMatcher.cpp
  GapMatcher.cpp
  EndMatcher.cpp
  CenterMatcher.cpp
)

set(matching_obj_INC
  AbstractMatcher.h
  OverlapMatcher.h
  GapMatcher.h
  EndMatcher.h
  CenterMatcher.h
)

add_library(MatchingObj OBJECT
  ${matching_obj_SRC}
  ${matching_obj_INC}
  )

enable_coverage_flags(MatchingObj)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)

