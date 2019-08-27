#=============================================================================
# Common functionality for all detector plugins
#=============================================================================

set(reduction_obj_SRC
  ChronoMerger.cpp
)

set(reduction_obj_INC
  ChronoMerger.h
)

add_library(ReductionObj OBJECT ${reduction_obj_SRC} ${reduction_obj_INC})
enable_coverage_flags(ReductionObj)

