include_directories(.)

set(JAL_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

get_filename_component(TEST_JSON_PATH "${JAL_BASE_DIR}/configs/v20_mappings.json" DIRECTORY)
get_filename_component(TEST_CSV_PATH "${JAL_BASE_DIR}/configs/sumo_voxel_map_20190711.csv" DIRECTORY)

#=============================================================================
# jalousie detector module and base test
#=============================================================================

set(JalousieBaseTest_INC
  ${JAL_BASE_DIR}/Config.h
  ${JAL_BASE_DIR}/Readout.h
  ${JAL_BASE_DIR}/SumoMappings.h
  ${JAL_BASE_DIR}/rapidcsv.h
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h)

set(JalousieBaseTest_SRC
  JalousieBaseTest.cpp
  ${JAL_BASE_DIR}/JalousieBase.cpp
  ${JAL_BASE_DIR}/Config.cpp
  ${JAL_BASE_DIR}/Readout.cpp
  ${JAL_BASE_DIR}/SumoMappings.cpp
  ${ESS_SOURCE_DIR}/common/testutils/SaveBuffer.cpp
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp)

create_test_executable(JalousieBaseTest)

target_compile_definitions(JalousieBaseTest
  PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}/")

target_compile_definitions(JalousieBaseTest
  PRIVATE TEST_CSV_PATH="${TEST_CSV_PATH}/")

#=============================================================================
# independent jalousie tests
#=============================================================================

set(JalReadoutTest_SRC ReadoutTest.cpp ${JAL_BASE_DIR}/Readout.cpp)
set(JalReadoutTest_INC ${JAL_BASE_DIR}/Readout.h)
create_test_executable(JalReadoutTest SKIP_MEMGRIND)

#=============================================================================
# tests depending on availability of reference data
#=============================================================================

set(TEST_DATA_PATH "${REFDATA}/jalousie/2019_07")
if(EXISTS ${TEST_DATA_PATH})

  #
  set(JalBuilderReadoutsTest_INC ${JAL_BASE_DIR}/BuilderReadouts.h ${JAL_BASE_DIR}/Readout.h
  ${JAL_BASE_DIR}/generators/CdtFile.h)
  set(JalBuilderReadoutsTest_SRC
    BuilderReadoutsTest.cpp
    ${JAL_BASE_DIR}/BuilderReadouts.cpp
    ${JAL_BASE_DIR}/Readout.cpp
    ${JAL_BASE_DIR}/generators/CdtFile.cpp
    )
    create_test_executable(JalBuilderReadoutsTest)
    target_compile_definitions(JalBuilderReadoutsTest
      PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")

  #
  set(JalSumoMappingsTest_INC ${JAL_BASE_DIR}/SumoMappings.h)
  set(JalSumoMappingsTest_SRC SumoMappingsTest.cpp ${JAL_BASE_DIR}/SumoMappings.cpp)
  create_test_executable(JalSumoMappingsTest)
  target_compile_definitions(JalSumoMappingsTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")

  #
  set(JalConfigTest_INC ${JAL_BASE_DIR}/Config.h ${JAL_BASE_DIR}/SumoMappings.h)
  set(JalConfigTest_SRC ConfigTest.cpp ${JAL_BASE_DIR}/Config.cpp ${JAL_BASE_DIR}/SumoMappings.cpp)
  create_test_executable(JalConfigTest)
  target_compile_definitions(JalConfigTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "ECDC: No Jalousie reference data found. Skipping JalCdtFileTest.")
  message(WARNING "ECDC: No Jalousie reference data found. Skipping JalBuilderReadoutsTest.")
  message(WARNING "ECDC: No Jalousie reference data found. Skipping JalSumoMappingsTest.")
endif()
