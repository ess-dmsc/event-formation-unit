#=============================================================================
# Classes for clustered event analysis
#=============================================================================

include_directories(.)
include_directories(${ESS_MODULE_DIR})

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

add_library(AnalysisLib OBJECT
  ${analysis_obj_SRC}
  ${analysis_obj_INC}
  )

target_link_libraries(AnalysisLib
  PRIVATE
    ${EFU_COMMON_LIBS}
)

enable_coverage_flags(AnalysisLib)

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
