#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(sonde_common_inc
  SoNDeBase.h
  ideas/Data.h
)
set(sonde_common_src
  SoNDeBase.cpp
  ideas/Data.cpp
)



# IDEA based readout for SoNDe detector
set(sonde_INC
  ${sonde_common_inc}
)
set(sonde_SRC
  SoNDe.cpp
  ${sonde_common_src}
)
create_module(sonde)


#=============================================================================
# Tools and Data generators
#=============================================================================


#=============================================================================
# Tests
#=============================================================================

set(SoNDeBaseTest_INC
  ${sonde_common_inc}
  ${ESS_SOURCE_DIR}/test/TestUDPServer.h
)
set(SoNDeBaseTest_SRC
  SoNDeBaseTest.cpp
  ${sonde_common_src}
  ${ESS_SOURCE_DIR}/test/TestUDPServer.cpp
)
create_test_executable(SoNDeBaseTest)


set(IDEASDataTest_SRC
  ideas/DataTest.cpp
  ideas/Data.cpp
  )
set(IDEASDataTest_INC
  ideas/Data.h
  ideas/TestData.h
  )
create_test_executable(IDEASDataTest)


set(IDEASGeometryTest_SRC
  ideas/GeometryTest.cpp)
set(IDEASGeometryTest_INC
  Geometry.h)
create_test_executable(IDEASGeometryTest)
