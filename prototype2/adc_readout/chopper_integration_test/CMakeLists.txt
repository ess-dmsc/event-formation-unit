set(AdcChopperInt_SRC
  AdcChopperIntegrationTest.cpp
  UDPServer.cpp
)

set(AdcChopperInt_INC
  UDPServer.h
)

add_executable(AdcChopperIntgerationTest ${AdcChopperInt_SRC} ${AdcChopperInt_INC})

target_include_directories(AdcChopperIntgerationTest PRIVATE ${ASIO_INCLUDE_DIR})

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
