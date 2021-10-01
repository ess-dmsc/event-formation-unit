#=============================================================================
# Tests
#=============================================================================

set(MGConfigTest_SRC ConfigTest.cpp ../Config.cpp)
set(MGConfigTest_INC ../Config.h)
set(MGConfigTest_LIB MgGeometryLib MgMesytecLib MgReductionLib)
create_test_executable(MGConfigTest)
target_compile_definitions(MGConfigTest
  PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}")

set(MultigridBaseTest_INC
  ../Config.h
  ../MultigridBase.h
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.h
  )
set(MultigridBaseTest_SRC
  MultigridBaseTest.cpp
  ../Config.cpp
  ../MultigridBase.cpp
  ${ESS_SOURCE_DIR}/common/testutils/TestUDPServer.cpp
  )
set(MultigridBaseTest_LIB MgGeometryLib MgMesytecLib MgReductionLib)
create_test_executable(MultigridBaseTest)
target_compile_definitions(MultigridBaseTest
  PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}")
