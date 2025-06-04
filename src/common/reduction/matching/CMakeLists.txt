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

add_library(MatchingLib OBJECT
  ${matching_obj_SRC}
  ${matching_obj_INC}
  )

target_link_libraries(MatchingLib
  PRIVATE
    ${EFU_COMMON_LIBS}
)

enable_coverage_flags(MatchingLib)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
