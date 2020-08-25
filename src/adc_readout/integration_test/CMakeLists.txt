set(AdcSimulator_SRC
  AdcSimulator.cpp
  DataPacket.cpp
  UdpConnection.cpp
  SampleRunGenerator.cpp
  SamplingTimer.cpp
)

set(AdcSimulator_INC
  DataPacket.h
  UdpConnection.h
  SampleRunGenerator.h
  SamplingTimer.h
)

set(AdcSimulator_ISPC_OBJ "")

find_package(Ispc)
ispc_compile_single_file(AdcSimulator_ISPC_OBJ SampleGen.ispc avx2)

find_package(Threads REQUIRED)

add_executable(AdcSimulator EXCLUDE_FROM_ALL ${AdcSimulator_SRC} ${AdcSimulator_INC} ${AdcSimulator_ISPC_OBJ})
set_target_properties(AdcSimulator PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_ispc_support(AdcSimulator)

target_include_directories(AdcSimulator PRIVATE "../" "../../common/")

target_link_libraries(AdcSimulator ${CMAKE_THREAD_LIBS_INIT} efu_common)

get_filename_component(ADC_CONFIG_FILE "AdcIntegrationTestConfig.ini" ABSOLUTE)
add_custom_command(
        TARGET AdcSimulator POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${ADC_CONFIG_FILE}
                "$<TARGET_FILE_DIR:efu>/")

set(AdcFileStreamer_SRC
        AdcFileStreamer.cpp
        WaveformData.cpp
        DataModulariser.cpp
        UdpConnection.cpp
        DataPacket.cpp
        SampleRunGenerator.cpp
        SamplingTimer.cpp)

set(AdcFileStreamer_INC
        WaveformData.h
        DataModulariser.h
        UdpConnection.h
        DataPacket.h
        ../AdcTimeStamp.h
        SampleRunGenerator.h
        SamplingTimer.h)

add_executable(AdcFileStreamer EXCLUDE_FROM_ALL ${AdcFileStreamer_SRC} ${AdcFileStreamer_INC})
set_target_properties(AdcFileStreamer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# HDF5 pulls in boost.
target_compile_definitions(AdcFileStreamer PRIVATE BOOST_DISABLE_ASSERTS=1)

target_include_directories(AdcFileStreamer PRIVATE "../" "../../common/")

target_link_libraries(AdcFileStreamer ${CMAKE_THREAD_LIBS_INIT} efu_common)
