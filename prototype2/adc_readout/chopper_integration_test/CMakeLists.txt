set(AdcChopperInt_SRC
  AdcChopperIntegrationTest.cpp
  UDPServer.cpp
  PacketGenerator.cpp
  TDCGenerator.cpp
  ../../common/Producer.cpp
)

set(AdcChopperInt_INC
  UDPServer.h
  PacketGenerator.h
  TDCGenerator.h
  f142_logdata_generated.h
  fwdi_forwarder_internal_generated.h
  ../../common/Producer.h
)

find_package(Threads REQUIRED)

add_executable(AdcChopperIntgerationTest ${AdcChopperInt_SRC} ${AdcChopperInt_INC})

target_include_directories(AdcChopperIntgerationTest PRIVATE ${ASIO_INCLUDE_DIR} "../" "../../common/")

target_link_libraries(AdcChopperIntgerationTest ${LibRDKafka_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

get_filename_component(ADC_CONFIG_FILE "AdcChopperIntegrationTestConfig.ini" ABSOLUTE)
add_custom_command(
        TARGET AdcChopperIntgerationTest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${ADC_CONFIG_FILE}
                "$<TARGET_FILE_DIR:efu2>/")

get_filename_component(DATA_TEMPLATE "DataPacketTemplate.dat" ABSOLUTE)
add_custom_command(
        TARGET AdcChopperIntgerationTest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${DATA_TEMPLATE}
                "$<TARGET_FILE_DIR:AdcChopperIntgerationTest>/")
