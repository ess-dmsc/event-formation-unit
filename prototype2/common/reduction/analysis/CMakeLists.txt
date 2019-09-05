#=============================================================================
# Classis for clustered event analysis
#=============================================================================

set(analysis_obj_SRC
  AbstractAnalyzer.cpp
  MgAnalyzer.cpp
  UtpcAnalyzer.cpp
  EventAnalyzer.cpp
)

set(analysis_obj_INC
  AbstractAnalyzer.h
  MgAnalyzer.h
  UtpcAnalyzer.h
  EventAnalyzer.h
)

add_library(AnalysisObj OBJECT
  ${analysis_obj_SRC}
  ${analysis_obj_INC}
  )

enable_coverage_flags(AnalysisObj)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)

