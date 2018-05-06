set(AdcChopperIntgerationTest_SRC
  AdcChopperIntegrationTest.cpp
  UDPServer.cpp
  PacketGenerator.cpp
  TDCGenerator.cpp
)

set(AdcChopperIntgerationTest_INC
  UDPServer.h
  PacketGenerator.h
  TDCGenerator.h
)

create_integration_test_executable(AdcChopperIntgerationTest)
target_include_directories(AdcChopperIntgerationTest PRIVATE "../")

get_filename_component(ADC_CONFIG_FILE "AdcChopperIntegrationTestConfig.ini" ABSOLUTE)
add_custom_command(
        TARGET AdcChopperIntgerationTest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${ADC_CONFIG_FILE}
                "$<TARGET_FILE_DIR:efu>/")

get_filename_component(DATA_TEMPLATE "DataPacketTemplate.dat" ABSOLUTE)
add_custom_command(
        TARGET AdcChopperIntgerationTest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${DATA_TEMPLATE}
                "$<TARGET_FILE_DIR:AdcChopperIntgerationTest>/")
