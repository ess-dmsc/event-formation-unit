# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Kafka related classes and tests
#=============================================================================

include_directories(.)

find_package(Trompeloeil REQUIRED)

set(ProducerTest_SRC test/ProducerTest.cpp)
set(ProducerTest_INC test/KafkaMocks.h)
create_test_executable(ProducerTest)
target_include_directories(ProducerTest PRIVATE ${Trompeloeil_INCLUDE_DIR})


set(EV44SerializerTest_SRC
  test/EV44SerializerTest.cpp
  )
create_test_executable(EV44SerializerTest)


set(AR52SerializerTest_SRC
  test/AR52SerializerTest.cpp
  )
create_test_executable(AR52SerializerTest)


get_filename_component(KAFKACONFIG_FILE "${ESS_COMMON_DIR}/kafka/kafka.json" ABSOLUTE)

set(KafkaConfigTest_SRC
  test/KafkaConfigTest.cpp
  )
create_test_executable(KafkaConfigTest)
target_compile_definitions(KafkaConfigTest PRIVATE KAFKACONFIG_FILE="${KAFKACONFIG_FILE}")
