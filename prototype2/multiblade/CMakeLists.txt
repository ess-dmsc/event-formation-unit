include_directories(.)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(caen_common_inc
  MBCaenBase.h
  ../common/clustering/Hit.h
  ../common/Hists.h
  ../common/HistSerializer.h
  caen/Readout.h
  caen/DataParser.h
  clustering/EventBuilder.h
  caen/Config.h
  )
set(caen_common_src
  MBCaenBase.cpp
  ../common/clustering/Hit.cpp
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  caen/Readout.cpp
  caen/DataParser.cpp
  clustering/EventBuilder.cpp
  caen/Config.cpp
  )

set(mbcaen_INC
  ${caen_common_inc}
)
set(mbcaen_SRC
  MBCaen.cpp
  ${caen_common_src}
)
create_module(mbcaen)

# Pipeline test
set(MBCAENBaseTest_INC
  ${caen_common_inc}
  ../../libs/include/Socket.h
  ../../libs/include/TSCTimer.h
  ../adc_readout/test/TestUDPServer.h
)
set(MBCAENBaseTest_SRC
  MBCaenBaseTest.cpp
  ${caen_common_src}
  ../../libs/source/Socket.cpp
  ../../libs/source/TSCTimer.cpp
  ../../libs/source/Timer.cpp
  ../adc_readout/test/TestUDPServer.cpp
)
create_test_executable(MBCAENBaseTest)

add_subdirectory(caen)
add_subdirectory(generators)
