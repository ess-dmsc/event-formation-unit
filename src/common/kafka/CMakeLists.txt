#=============================================================================
# Kafka related classesand tests
#=============================================================================

include_directories(.)

find_package(Trompeloeil REQUIRED)

set(ProducerTest_SRC test/ProducerTest.cpp)
set(ProducerTest_INC test/KafkaMocks.h)
create_test_executable(ProducerTest)
target_include_directories(ProducerTest PRIVATE ${Trompeloeil_INCLUDE_DIR})


set(EV42SerializerTest_SRC
  test/EV42SerializerTest.cpp
  )
create_test_executable(EV42SerializerTest)


set(EV44SerializerTest_SRC
  test/EV44SerializerTest.cpp
  )
create_test_executable(EV44SerializerTest)


get_filename_component(KAFKACONFIG_FILE "${ESS_COMMON_DIR}/kafka/kafka.json" ABSOLUTE)

set(KafkaConfigTest_SRC
  test/KafkaConfigTest.cpp
  )
create_test_executable(KafkaConfigTest)
target_compile_definitions(KafkaConfigTest PRIVATE KAFKACONFIG_FILE="${KAFKACONFIG_FILE}")
