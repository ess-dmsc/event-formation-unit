#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

# Two-thread implementation of Multi Grid demonstrator detector
# for Mesytec Readout
set(multigrid_common_src
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  ../common/ReadoutSerializer.cpp
  mesytec/BuilderMesytec.cpp
  mesytec/Vmmr16Parser.cpp
  mesytec/Sis3153Parser.cpp
  mesytec/Readout.cpp
  mesytec/BuilderReadouts.cpp
  reduction/Reduction.cpp
  reduction/EventAnalysis.cpp
  Config.cpp
  )
set(multigrid_common_inc
  ../common/Hists.h
  ../common/HistSerializer.h
  ../common/ReadoutSerializer.h
  geometry/SequoiaGeometry.h
  geometry/BusGeometry.h
  mesytec/BuilderMesytec.h
  mesytec/Vmmr16Parser.h
  mesytec/Sis3153Parser.h
  mesytec/Readout.h
  mesytec/BuilderReadouts.h
  reduction/Reduction.h
  reduction/EventAnalysis.h
  AbstractBuilder.h
  Config.h
  )

set(multigrid_INC
  MultigridBase.h
  ${multigrid_common_inc}
)

set(multigrid_SRC
  Multigrid.cpp
  MultigridBase.cpp
  ${multigrid_common_src}
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

set(MGConfigTest_SRC
  ${multigrid_common_src}
  ConfigTest.cpp
  )
set(MGConfigTest_INC
  ${multigrid_common_inc}
  )
create_test_executable(MGConfigTest)
target_compile_definitions(MGConfigTest
  PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}")

set(MultigridBaseTest_INC
  ${multigrid_common_inc}
  MultigridBase.h
  ../../libs/include/Socket.h
  ../../libs/include/TSCTimer.h
  ../adc_readout/test/TestUDPServer.h
)
set(MultigridBaseTest_SRC
  ${multigrid_common_src}
  MultigridBase.cpp
  MultigridBaseTest.cpp
  ../../libs/source/Socket.cpp
  ../../libs/source/TSCTimer.cpp
  ../adc_readout/test/TestUDPServer.cpp
)
create_test_executable(MultigridBaseTest)
target_compile_definitions(MultigridBaseTest
  PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}")
