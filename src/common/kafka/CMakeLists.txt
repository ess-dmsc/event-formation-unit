# Copyright (C) 2021 - 2023 European Spallation Source, ERIC. See LICENSE file

#=============================================================================
# Kafka related classes and tests
#=============================================================================

include_directories(.)

find_package(FakeIt REQUIRED)

set(ProducerTest_SRC test/ProducerTest.cpp)
set(ProducerTest_INC test/KafkaMocks.h)
create_test_executable(ProducerTest)
target_include_directories(ProducerTest PRIVATE ${FakeIt_INCLUDE_DIR})

# Add compiler-specific options for ProducerTest
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Options specific to Clang compiler
  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_options(ProducerTest PRIVATE -Wno-cast-function-type-mismatch
    -Wno-missing-template-arg-list-after-template-kw)
    message(STATUS "ECDC: To compile FakeIt mocks, we set -Wno-cast-function-type-mismatch -Wno-missing-template-arg-list-after-template-kw for clang")
  else()
    target_compile_options(ProducerTest PRIVATE -Wno-cast-function-type-strict
    -Wno-gnu-string-literal-operator-template)
    message(STATUS "ECDC: To compile FakeIt mocks, we set -Wno-cast-function-type-strict -Wno-gnu-string-literal-operator-template for clang")
  endif()
endif()

set(EV44SerializerTest_SRC
  test/EV44SerializerTest.cpp
  )
create_test_executable(EV44SerializerTest)

set(AR51SerializerTest_SRC
  test/AR51SerializerTest.cpp
  )
create_test_executable(AR51SerializerTest)

get_filename_component(KAFKACONFIG_FILE "${ESS_COMMON_DIR}/kafka/kafka.json" ABSOLUTE)

set(KafkaConfigTest_SRC
  test/KafkaConfigTest.cpp
  )
create_test_executable(KafkaConfigTest)
target_compile_definitions(KafkaConfigTest PRIVATE KAFKACONFIG_FILE="${KAFKACONFIG_FILE}")
