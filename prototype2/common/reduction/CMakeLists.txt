#=============================================================================
# Common functionality for all detector plugins
#=============================================================================

set(analysis_obj_SRC
  ReducedEvent.cpp
  AbstractAnalyzer.cpp
  MgAnalyzer.cpp
  UtpcAnalyzer.cpp
  NeutronEvent.cpp
  ChronoMerger.cpp
)

set(analysis_obj_INC
  ReducedEvent.h
  AbstractAnalyzer.h
  MgAnalyzer.h
  UtpcAnalyzer.h
  NeutronEvent.h
  ChronoMerger.h
)

add_library(AnalysisObj OBJECT ${analysis_obj_SRC} ${analysis_obj_INC})
enable_coverage_flags(AnalysisObj)

add_subdirectory(test)

