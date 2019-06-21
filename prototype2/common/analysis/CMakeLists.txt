#=============================================================================
# Common functionality for all detector plugins
#=============================================================================

set(analysis_obj_SRC
  AbstractAnalyzer.cpp
  MGAnalysis.cpp
  uTPC.cpp
)

set(analysis_obj_INC
  AbstractAnalyzer.h
  MGAnalysis.h
  uTPC.h
)

add_library(AnalysisObj OBJECT ${analysis_obj_SRC} ${analysis_obj_INC})
enable_coverage_flags(AnalysisObj)

add_subdirectory(test)

