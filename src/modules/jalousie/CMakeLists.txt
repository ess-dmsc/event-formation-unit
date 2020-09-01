
add_subdirectory(generators)

include_directories(.)

get_filename_component(TEST_JSON_PATH "configs/v20_mappings.json" DIRECTORY)
get_filename_component(TEST_CSV_PATH "configs/sumo_voxel_map_20190711.csv" DIRECTORY)

#=============================================================================
# jalousie detector module and base test
#=============================================================================

set(jalousie_common_inc Config.h Readout.h SumoMappings.h rapidcsv.h)
set(jalousie_common_src Config.cpp Readout.cpp SumoMappings.cpp)

set(jalousie_INC JalousieBase.h ${jalousie_common_inc})
set(jalousie_SRC JalousieBase.cpp Jalousie.cpp ${jalousie_common_src})
create_module(jalousie)

#
set(JalousieBaseTest_INC
  ${jalousie_common_inc}
  ${ESS_SOURCE_DIR}/test/SaveBuffer.h
  ${ESS_SOURCE_DIR}/test/TestUDPServer.h)
set(JalousieBaseTest_SRC
  JalousieBase.cpp
  JalousieBaseTest.cpp
  ${jalousie_common_src}
  ${ESS_SOURCE_DIR}/test/SaveBuffer.cpp
  ${ESS_SOURCE_DIR}/test/TestUDPServer.cpp)
create_test_executable(JalousieBaseTest)
target_compile_definitions(JalousieBaseTest
  PRIVATE TEST_JSON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_JSON_PATH}/")
target_compile_definitions(JalousieBaseTest
PRIVATE TEST_CSV_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_CSV_PATH}/")

#=============================================================================
# independent jalousie tests
#=============================================================================

set(JalReadoutTest_SRC ReadoutTest.cpp Readout.cpp)
set(JalReadoutTest_INC Readout.h)
create_test_executable(JalReadoutTest SKIP_MEMGRIND)

#=============================================================================
# tests depending on availability of reference data
#=============================================================================

set(TEST_DATA_PATH "${REFDATA}/jalousie/2019_07")
if(EXISTS ${TEST_DATA_PATH})

  set(JalCdtFileTest_INC generators/CdtFile.h Readout.h)
  set(JalCdtFileTest_SRC
    generators/CdtFileTest.cpp
    generators/CdtFile.cpp
    Readout.cpp
    )
  create_test_executable(JalCdtFileTest)
  target_compile_definitions(JalCdtFileTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")

  #
  set(JalBuilderReadoutsTest_INC BuilderReadouts.h Readout.h
    generators/CdtFile.h)
  set(JalBuilderReadoutsTest_SRC
    BuilderReadoutsTest.cpp
    BuilderReadouts.cpp
    Readout.cpp
    generators/CdtFile.cpp
    )
    create_test_executable(JalBuilderReadoutsTest)
    target_compile_definitions(JalBuilderReadoutsTest
      PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")

  #
  set(JalSumoMappingsTest_INC SumoMappings.h)
  set(JalSumoMappingsTest_SRC SumoMappingsTest.cpp SumoMappings.cpp)
  create_test_executable(JalSumoMappingsTest)
  target_compile_definitions(JalSumoMappingsTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")

  #
  set(JalConfigTest_INC Config.h SumoMappings.h)
  set(JalConfigTest_SRC ConfigTest.cpp Config.cpp SumoMappings.cpp)
  create_test_executable(JalConfigTest)
  target_compile_definitions(JalConfigTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "ECDC: No Jalousie reference data found. Skipping JalCdtFileTest.")
  message(WARNING "ECDC: No Jalousie reference data found. Skipping JalBuilderReadoutsTest.")
  message(WARNING "ECDC: No Jalousie reference data found. Skipping JalSumoMappingsTest.")
endif()
