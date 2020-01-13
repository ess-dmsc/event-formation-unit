include_directories(.)

#=============================================================================
# Detector Pipeline Plugins
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

set(loki_INC
  ${loki_common_inc}
)
set(loki_SRC
  Loki.cpp
  ${loki_common_src}
)
create_module(loki)

##
## Pipeline test
##
set(LokiBaseTest_INC
  ${loki_common_inc}
  ../test/SaveBuffer.h
  ../adc_readout/test/TestUDPServer.h
)
set(LokiBaseTest_SRC
  test/LokiBaseTest.cpp
  ${loki_common_src}
  ../test/SaveBuffer.cpp
  ../adc_readout/test/TestUDPServer.cpp
)
create_test_executable(LokiBaseTest)

add_subdirectory(generators)
add_subdirectory(test)
