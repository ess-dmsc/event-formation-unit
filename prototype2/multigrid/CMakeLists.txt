
# Where does this really need to go ???
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/calib_data
  DESTINATION ${CMAKE_BINARY_DIR}/prototype2)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

# Two-thread implementation of Multi Grid demonstrator detector
# for Mesytec Readout
set(mgmesytec_common_src
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  ../common/ReadoutSerializer.cpp
  parsers/Vmmr16Parser.cpp
  parsers/Sis3153Parser.cpp
  parsers/Hit.cpp
  mgmesytec/EfuMaximum.cpp
  mgmesytec/EfuCenterMass.cpp
  mgmesytec/EfuPrioritized.cpp
  MgConfig.cpp
  MGMesytecBase.cpp
  )
set(mgmesytec_common_inc
  ../common/Hists.h
  ../common/HistSerializer.h
  ../common/ReadoutSerializer.h
  parsers/Vmmr16Parser.h
  parsers/Sis3153Parser.h
  parsers/Hit.h
  mgmesytec/Efu.h
  mgmesytec/EfuMaximum.h
  mgmesytec/EfuCenterMass.h
  mgmesytec/EfuPrioritized.h
  mgmesytec/SequoiaGeometry.h
  mgmesytec/BusGeometry.h
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

add_subdirectory(parsers)
add_subdirectory(mgmesytec)

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
