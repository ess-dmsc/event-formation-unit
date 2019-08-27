#=============================================================================
# Classes for Multi-grid event reduction pipeline
#=============================================================================

set(mg_reduction_obj_SRC
  ModuleGeometry.cpp
  EventProcessingStats.cpp
  ModulePipeline.cpp
  Reduction.cpp
  )

set(mg_reduction_obj_INC
  ModuleGeometry.h
  EventProcessingStats.h
  ModulePipeline.h
  Reduction.h
  )

add_library(MgReduction STATIC
  ${mg_reduction_obj_SRC}
  ${mg_reduction_obj_INC}
  )

enable_coverage_flags(MgReduction)
