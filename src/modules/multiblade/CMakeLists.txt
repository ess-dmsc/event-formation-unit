
add_subdirectory(caen)
add_subdirectory(generators)
add_subdirectory(clustering)

include_directories(.)

#=============================================================================
# multiblade detector module and base test
#=============================================================================
set(caen_common_inc
  MBCaenBase.h
  caen/Readout.h
  caen/DataParser.h
  clustering/EventBuilder.h
  caen/Config.h
  )
set(caen_common_src
  MBCaenBase.cpp
  caen/Readout.cpp
  caen/DataParser.cpp
  clustering/EventBuilder.cpp
  caen/Config.cpp
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
