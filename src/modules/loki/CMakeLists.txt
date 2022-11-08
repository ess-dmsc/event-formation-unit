
add_subdirectory(generators)
add_subdirectory(test)

include_directories(.)

#=============================================================================
# loki detector module and base test
#=============================================================================
set(loki_common_inc
  Counters.h
  LokiBase.h
  LokiInstrument.h
  geometry/PanelGeometry.h
  geometry/Config.h
  geometry/Calibration.h
  readout/DataParser.h
  )
set(loki_common_src
  LokiBase.cpp
  LokiInstrument.cpp
  geometry/Config.cpp
  geometry/Calibration.cpp
  readout/DataParser.cpp
  )
set(loki_INC ${loki_common_inc})
set(loki_SRC
  ${loki_common_src}
  main.cpp
  )
set(loki_LIB efu_essreadout)
create_executable(loki)

add_dependencies(loki efu_essreadout)
target_compile_definitions(loki PUBLIC EFU_LOKI)

#
set(LokiBaseTest_INC
  ${loki_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
)
set(LokiBaseTest_SRC
  test/LokiBaseTest.cpp
  ${loki_common_src}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
)
create_test_executable(LokiBaseTest)

##
## LokiInstrumentTest Module integration test
##
set(LokiInstrumentTest_INC
  ${loki_common_inc}
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  )
set(LokiInstrumentTest_SRC
  ${loki_common_src}
  test/LokiInstrumentTest.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
)
create_test_executable(LokiInstrumentTest)
