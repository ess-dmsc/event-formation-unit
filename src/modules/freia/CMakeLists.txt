set(TEST_DATA_PATH "${REFDATA}/multiblade/2020")
if(EXISTS ${TEST_DATA_PATH})
  add_definitions("-DHAS_REFDATA")
  message(STATUS "ECDC: HAS multiblade reference data, using large datasets")
else()
  message(STATUS "ECDC: No multiblade reference data, skipping large datasets")
endif()

add_subdirectory(caen)
#add_subdirectory(generators)
add_subdirectory(clustering)

include_directories(.)

#=============================================================================
# multiblade detector module and base test
#=============================================================================
set(caen_common_inc
  MBCaenBase.h
  MBCaenInstrument.h
  clustering/EventBuilder.h
  ../readout/common/ReadoutParser.h
  ../readout/vmm3/VMM3Parser.h
  )
set(caen_common_src
  MBCaenBase.cpp
  MBCaenInstrument.cpp
  clustering/EventBuilder.cpp
  ../readout/common/ReadoutParser.cpp
  ../readout/vmm3/VMM3Parser.cpp
  )

set(mbcaen_INC ${caen_common_inc})
set(mbcaen_SRC MBCaen.cpp ${caen_common_src})
create_module(mbcaen)

#
set(MBCAENBaseTest_INC
  ${caen_common_inc}
  ${ESS_SOURCE_DIR}/test/SaveBuffer.h
  ${ESS_SOURCE_DIR}/test/TestUDPServer.h
)
set(MBCAENBaseTest_SRC
  MBCaenBaseTest.cpp
  ${ESS_SOURCE_DIR}/test/SaveBuffer.cpp
  ${caen_common_src}
  ${ESS_SOURCE_DIR}/test/TestUDPServer.cpp
)
create_test_executable(MBCAENBaseTest)
