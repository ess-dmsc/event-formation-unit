
add_subdirectory(test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(DataPipeline DataPipeline.cpp)