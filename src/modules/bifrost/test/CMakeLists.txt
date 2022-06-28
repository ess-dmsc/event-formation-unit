include_directories(.)

set(BIFROST_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

#=============================================================================
# independent bifrost unit tests
#=============================================================================

set(BifrostGeometryTest_INC
  ${BIFROST_BASE_DIR}/geometry/Geometry.h)

set(BifrostGeometryTest_SRC
  ${BIFROST_BASE_DIR}/geometry/Geometry.cpp
  BifrostGeometryTest.cpp)

create_test_executable(BifrostGeometryTest)
