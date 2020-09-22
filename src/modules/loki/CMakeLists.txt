
add_subdirectory(generators)
add_subdirectory(test)

include_directories(.)

#=============================================================================
# loki detector module and base test
#=============================================================================
set(loki_common_inc
  LokiBase.h
  Counters.h
  geometry/PanelGeometry.h
  geometry/Config.h
  geometry/Calibration.h
  readout/DataParser.h
  ../readout/ReadoutParser.h
  )
set(loki_common_src
  LokiBase.cpp
  geometry/Config.cpp
  geometry/Calibration.cpp
  readout/DataParser.cpp
  ../readout/ReadoutParser.cpp
  )
set(loki_INC ${loki_common_inc})
set(loki_SRC Loki.cpp ${loki_common_src})
create_module(loki)

#
set(LokiBaseTest_INC
  ${loki_common_inc}
  ${ESS_SOURCE_DIR}/test/SaveBuffer.h
  ${ESS_SOURCE_DIR}/test/TestUDPServer.h
)
set(LokiBaseTest_SRC
  test/LokiBaseTest.cpp
  ${loki_common_src}
  ${ESS_SOURCE_DIR}/test/SaveBuffer.cpp
  ${ESS_SOURCE_DIR}/test/TestUDPServer.cpp
)
create_test_executable(LokiBaseTest)
