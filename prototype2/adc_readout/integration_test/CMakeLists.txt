set(AdcSimulator_SRC
  AdcSimulator.cpp
  UDPServer.cpp
  PacketGenerator.cpp
  ../../common/Producer.cpp
)

set(AdcSimulator_INC
  UDPServer.h
  PacketGenerator.h
  ../../common/Producer.h
)

find_package(Threads REQUIRED)

add_executable(AdcSimulator EXCLUDE_FROM_ALL ${AdcSimulator_SRC} ${AdcSimulator_INC})

target_include_directories(AdcSimulator PRIVATE ${ASIO_INCLUDE_DIR} "../" "../../common/")

# target_link_libraries(AdcSimulator ${LibRDKafka_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

get_filename_component(ADC_CONFIG_FILE "AdcIntegrationTestConfig.ini" ABSOLUTE)
add_custom_command(
        TARGET AdcSimulator POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${ADC_CONFIG_FILE}
                "$<TARGET_FILE_DIR:efu>/")

get_filename_component(DATA_TEMPLATE_PATH "DataPacketTemplate.dat" DIRECTORY)
target_compile_definitions(AdcSimulator
    PRIVATE DATA_TEMPLATE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${DATA_TEMPLATE_PATH}/")
