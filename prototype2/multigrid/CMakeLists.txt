#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

# Two-thread implementation of Multi Grid demonstrator detector
# for Mesytec Readout
set(mgmesytec_common_src
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  ../common/ReadoutSerializer.cpp
  mesytec/Vmmr16Parser.cpp
  mesytec/Sis3153Parser.cpp
  mesytec/Readout.cpp
  generators/BuilderReadouts.cpp
  reduction/Reduction.cpp
  reduction/EventAnalysis.cpp
  MgConfig.cpp
  )
set(mgmesytec_common_inc
  ../common/Hists.h
  ../common/HistSerializer.h
  ../common/ReadoutSerializer.h
  geometry/SequoiaGeometry.h
  geometry/BusGeometry.h
  mesytec/Vmmr16Parser.h
  mesytec/Sis3153Parser.h
  mesytec/Readout.h
  generators/BuilderReadouts.h
  reduction/Reduction.h
  reduction/EventAnalysis.h
  AbstractBuilder.h
  MgConfig.h
  )

set(mgmesytec_INC
  MGMesytecBase.h
  ${mgmesytec_common_inc}
)

set(mgmesytec_SRC
  MGMesytec.cpp
  MGMesytecBase.cpp
  ${mgmesytec_common_src}
)
create_module(mgmesytec)


#=============================================================================
# Tests
#=============================================================================

get_filename_component(TEST_JSON_PATH "configs/ILL_mappings.json" DIRECTORY)
set(TEST_JSON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_JSON_PATH}/")

add_subdirectory(geometry)
add_subdirectory(mesytec)
add_subdirectory(generators)
add_subdirectory(reduction)

set(MgConfigTest_SRC
  ${mgmesytec_common_src}
  MgConfigTest.cpp
  )
set(MgConfigTest_INC
  ${mgmesytec_common_inc}
  )
create_test_executable(MgConfigTest)
target_compile_definitions(MgConfigTest
  PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}")

set(MGMesytecBaseTest_INC
  ${mgmesytec_common_inc}
  MGMesytecBase.h
  ../../libs/include/Socket.h
  ../../libs/include/TSCTimer.h
  ../adc_readout/test/TestUDPServer.h
)
set(MGMesytecBaseTest_SRC
  ${mgmesytec_common_src}
  MGMesytecBase.cpp
  MGMesytecBaseTest.cpp
  ../../libs/source/Socket.cpp
  ../../libs/source/TSCTimer.cpp
  ../adc_readout/test/TestUDPServer.cpp
)
create_test_executable(MGMesytecBaseTest)
