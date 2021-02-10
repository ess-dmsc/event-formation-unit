include_directories(.)

set(JAL_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

#=============================================================================
# independent jalousie tests
#=============================================================================

set(DreamGeometryTest_SRC DreamGeometryTest.cpp)
set(DreamGeometryTest_INC ${JAL_BASE_DIR}/geometry/DreamGeometry.h)
create_test_executable(DreamGeometryTest)
