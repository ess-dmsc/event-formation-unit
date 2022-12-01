#=============================================================================
# Classes for multigrid ModuleGeometry
#=============================================================================

include_directories(.)
include_directories(${ESS_SOURCE_DIR})

set(MGModuleGeomTest_INC
  ModuleGeometry.h
  )

set(MGModuleGeomTest_SRC
  ModuleGeometry.cpp
  test/MGModuleGeomTest.cpp
  )

create_test_executable(MGModuleGeomTest)
