#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

# Two-thread implementation of Multi Grid demonstrator detector
# for Mesytec Readout
set(mgmesytec_common_src
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  ../common/ReadoutSerializer.cpp
  mgmesytec/Vmmr16Parser.cpp
  mgmesytec/Sis3153Parser.cpp
  mgmesytec/MesytecReadout.cpp
  reduction/EfuMaximum.cpp
  reduction/EfuCenterMass.cpp
  reduction/EfuPrioritized.cpp
  MgConfig.cpp
  MGMesytecBase.cpp
  )
set(mgmesytec_common_inc
  ../common/Hists.h
  ../common/HistSerializer.h
  ../common/ReadoutSerializer.h
  geometry/SequoiaGeometry.h
  geometry/BusGeometry.h
  mgmesytec/Vmmr16Parser.h
  mgmesytec/Sis3153Parser.h
  mgmesytec/MesytecReadout.h
  reduction/Efu.h
  reduction/EfuMaximum.h
  reduction/EfuCenterMass.h
  reduction/EfuPrioritized.h
  AbstractBuilder.h
  MGMesytecBase.h
  MgConfig.h
  )

set(mgmesytec_INC
  ${mgmesytec_common_inc}
)
set(mgmesytec_SRC
  MGMesytec.cpp
  ${mgmesytec_common_src}
)
create_module(mgmesytec)


#=============================================================================
# Tests
#=============================================================================

add_subdirectory(geometry)
add_subdirectory(mgmesytec)
add_subdirectory(reduction)

get_filename_component(TEST_JSON_PATH "configs/ILL_mappings.json" DIRECTORY)

set(MgConfigTest_SRC
  MgConfig.cpp
  MgConfigTest.cpp
  ../../prototype2/common/DataSave.cpp
  )
set(MgConfigTest_INC
  MgConfig.h
  ../../prototype2/common/DataSave.h
  )
create_test_executable(MgConfigTest)
target_compile_definitions(MgConfigTest
  PRIVATE TEST_JSON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_JSON_PATH}/")

set(MGMesytecBaseTest_INC
  ${mgmesytec_common_inc}
  ../../libs/include/Socket.h
  ../../libs/include/TSCTimer.h
  ../adc_readout/test/TestUDPServer.h
)
set(MGMesytecBaseTest_SRC
  ${mgmesytec_common_src}
  MGMesytecBaseTest.cpp
  ../../libs/source/Socket.cpp
  ../../libs/source/TSCTimer.cpp
  ../adc_readout/test/TestUDPServer.cpp
)
create_test_executable(MGMesytecBaseTest)
