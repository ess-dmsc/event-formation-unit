#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

set(pipeline_common_source
  GdGemBase.cpp
  NMXConfig.cpp
  nmx/uTPC.cpp
  ../common/clustering/Hit.cpp
  ../common/clustering/Cluster.cpp
  ../common/clustering/Event.cpp
  ../common/clustering/GapClusterer.cpp
  ../common/clustering/GapMatcher.cpp
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  nmx/TrackSerializer.cpp
  srs/SRSMappings.cpp
  srs/SRSTime.cpp
  vmm3/ParserVMM3.cpp
  vmm3/BuilderVMM3.cpp
  vmm3/CalibrationFile.cpp  # Also works for VMM2
  )

set(pipeline_common_inc
  GdGemBase.h
  NMXConfig.h
  nmx/uTPC.h
  ../common/clustering/Hit.h
  ../common/clustering/Cluster.h
  ../common/clustering/Event.h
  ../common/clustering/GapClusterer.h
  ../common/clustering/GapMatcher.h
  ../common/clustering/AbstractClusterer.h
  ../common/Hists.h
  nmx/Readout.h
  ../common/HistSerializer.h
  nmx/TrackSerializer.h
  nmx/AbstractBuilder.h
  srs/SRSMappings.h
  srs/SRSTime.h
  vmm3/ParserVMM3.h
  vmm3/BuilderVMM3.h
  vmm3/CalibrationFile.h # also works for VMM2
  )

set(gdgem_SRC GdGem.cpp ${GdGemBase_SRC} ${pipeline_common_source})
set(gdgem_INC ${pipeline_common_inc})
create_module(gdgem)

#
# Unit Tests
#

get_filename_component(TEST_JSON_PATH "configs/vmm3.json" DIRECTORY)

set(NMXConfigTest_INC
  NMXConfig.h
  nmx/uTPC.h
  srs/SRSTime.h
  srs/SRSMappings.h
  vmm3/CalibrationFile.cpp
  )
set(NMXConfigTest_SRC
  NMXConfigTest.cpp
  NMXConfig.cpp
  srs/SRSTime.cpp
  srs/SRSMappings.cpp
  vmm3/CalibrationFile.cpp
  nmx/uTPC.cpp
  ../common/clustering/Event.cpp
  )
create_test_executable(NMXConfigTest)
target_compile_definitions(NMXConfigTest
  PRIVATE TEST_JSON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_JSON_PATH}/")

set(GdGemBaseTest_INC
  ${pipeline_common_inc}
  ../../libs/include/Socket.h
  ../../libs/include/TSCTimer.h
  ../adc_readout/test/TestUDPServer.h)
set(GdGemBaseTest_SRC
  GdGemBaseTest.cpp
  ${pipeline_common_source}
  ../../libs/source/Socket.cpp
  ../../libs/source/TSCTimer.cpp
  ../adc_readout/test/TestUDPServer.cpp)
create_test_executable(GdGemBaseTest)
target_compile_definitions(GdGemBaseTest
  PRIVATE TEST_JSON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_JSON_PATH}/")

#=============================================================================
# Component tests
#=============================================================================


add_subdirectory(nmx)
add_subdirectory(srs)
add_subdirectory(vmm3)
add_subdirectory(clustering)
add_subdirectory(generators)
