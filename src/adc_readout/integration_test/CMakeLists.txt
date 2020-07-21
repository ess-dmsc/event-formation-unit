set(AdcSimulator_SRC
  AdcSimulator.cpp
  AmpEventDelay.cpp
  ContinousSamplingTimer.cpp
  DataPacket.cpp
  UdpConnection.cpp
  PoissonDelay.cpp
  SampleRunGenerator.cpp
  SamplingTimer.cpp
)

set(AdcSimulator_INC
  AmpEventDelay.h
  ContinousSamplingTimer.h
  DataPacket.h
  UdpConnection.h
  PoissonDelay.h
  SampleRunGenerator.h
  SamplingTimer.h
)


add_custom_command(OUTPUT SampleGen.o
                   COMMAND ispc --target=avx2 ${CMAKE_CURRENT_LIST_DIR}/SampleGen.ispc -o SampleGen.o
                   DEPENDS ${CMAKE_CURRENT_LIST_DIR}/SampleGen.ispc)


find_package(Threads REQUIRED)

add_executable(AdcSimulator EXCLUDE_FROM_ALL ${AdcSimulator_SRC} ${AdcSimulator_INC} SampleGen.o)
set_target_properties(AdcSimulator PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_include_directories(AdcSimulator PRIVATE ${ASIO_INCLUDE_DIR} "../" "../../common/")

target_link_libraries(AdcSimulator ${CMAKE_THREAD_LIBS_INIT} efu_common $<TARGET_OBJECTS:AdcReadout>)

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
        PoissonDelay.cpp
        UdpConnection.cpp
        DataPacket.cpp
        ../AdcTimeStamp.cpp
        SamplingTimer.cpp)

set(AdcFileStreamer_INC
        WaveformData.h
        DataModulariser.h
        PoissonDelay.h
        UdpConnection.h
        DataPacket.h
        ../AdcTimeStamp.h
        SamplingTimer.h)

add_executable(AdcFileStreamer EXCLUDE_FROM_ALL ${AdcFileStreamer_SRC} ${AdcFileStreamer_INC})
set_target_properties(AdcFileStreamer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_compile_definitions(AdcFileStreamer PRIVATE BOOST_DISABLE_ASSERTS=1 ASIO_HAS_BOOST_ASSERT=1)

target_include_directories(AdcFileStreamer PRIVATE ${ASIO_INCLUDE_DIR} "../" "../../common/")

target_link_libraries(AdcFileStreamer ${CMAKE_THREAD_LIBS_INIT} ${EFU_COMMON_LIBS})
