#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

set(pipeline_common_source
  GdGemBase.cpp
  NMXConfig.cpp
  nmx/AbstractAnalyzer.cpp
  nmx/uTPC.cpp
  nmx/MGAnalysis.cpp
  nmx/TrackSerializer.cpp
  srs/SRSMappings.cpp
  srs/SRSTime.cpp
  srs/ParserVMM3.cpp
  srs/BuilderVMM3.cpp
  srs/CalibrationFile.cpp  # Also works for VMM2
  ../multigrid/geometry/ModuleGeometry.cpp
  ../multigrid/geometry/Filter.cpp
  generators/BuilderReadouts.cpp
  generators/BuilderHits.cpp
  )

set(pipeline_common_inc
  GdGemBase.h
  NMXConfig.h
  nmx/AbstractAnalyzer.h
  nmx/uTPC.h
  nmx/MGAnalysis.h
  nmx/Readout.h
  nmx/TrackSerializer.h
  nmx/AbstractBuilder.h
  srs/SRSMappings.h
  srs/SRSTime.h
  srs/ParserVMM3.h
  srs/BuilderVMM3.h
  srs/CalibrationFile.h # also works for VMM2
  ../multigrid/geometry/ModuleGeometry.h
  ../multigrid/geometry/Filter.h
  generators/BuilderReadouts.h
  generators/BuilderHits.h
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
  nmx/AbstractAnalyzer.h
  nmx/uTPC.h
  nmx/MGAnalysis.h
  srs/SRSTime.h
  srs/SRSMappings.h
  srs/CalibrationFile.cpp
  ../multigrid/geometry/ModuleGeometry.cpp
  ../multigrid/geometry/Filter.cpp
  )
set(NMXConfigTest_SRC
  NMXConfigTest.cpp
  NMXConfig.cpp
  srs/SRSTime.cpp
  srs/SRSMappings.cpp
  srs/CalibrationFile.cpp
  ../multigrid/geometry/ModuleGeometry.h
  ../multigrid/geometry/Filter.h
  nmx/AbstractAnalyzer.cpp
  nmx/uTPC.cpp
  nmx/MGAnalysis.cpp
  )
create_test_executable(NMXConfigTest)
target_compile_definitions(NMXConfigTest
  PRIVATE TEST_JSON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_JSON_PATH}/")

set(GdGemBaseTest_INC
  ${pipeline_common_inc}
  ../adc_readout/test/TestUDPServer.h)
set(GdGemBaseTest_SRC
  GdGemBaseTest.cpp
  ${pipeline_common_source}
  ../adc_readout/test/TestUDPServer.cpp)
create_test_executable(GdGemBaseTest)
target_compile_definitions(GdGemBaseTest
  PRIVATE TEST_JSON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_JSON_PATH}/")

#=============================================================================
# Component tests
#=============================================================================

add_subdirectory(nmx)
add_subdirectory(srs)
add_subdirectory(tests)
add_subdirectory(generators)
