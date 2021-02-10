include_directories(.)

set(DREAM_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

#=============================================================================
# independent dream unit tests
#=============================================================================

set(DreamGeometryTest_SRC DreamGeometryTest.cpp)
set(DreamGeometryTest_INC ${DREAM_BASE_DIR}/geometry/DreamGeometry.h)
create_test_executable(DreamGeometryTest)


#=============================================================================
# independent dream benchmark tests
#=============================================================================

set(DreamBenchmarkTest_SRC DreamBenchmarkTest.cpp)
set(DreamBenchmarkTest_INC ${DREAM_BASE_DIR}/geometry/DreamGeometry.h)
create_benchmark_executable(DreamBenchmarkTest)
