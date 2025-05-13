#=============================================================================
# Common functionality for all detector plugins
#=============================================================================

add_subdirectory(monitor)
add_subdirectory(kafka)
add_subdirectory(reduction)
add_subdirectory(time)
add_subdirectory(readout)
add_subdirectory(kafka/serializer)

set(efu_common_SRC
  config/Config.cpp
  debug/Hexdump.cpp
  debug/Log.cpp
  detector/EFUArgs.cpp
  kafka/EV44Serializer.cpp
  kafka/AR51Serializer.cpp
  kafka/KafkaConfig.cpp
  kafka/Producer.cpp
  kafka/serializer/AbstractSerializer.cpp
  system/Socket.cpp
  Statistics.cpp
  StatPublisher.cpp
  ${ESS_SOURCE_DIR}/efu/ExitHandler.cpp
  ${ESS_SOURCE_DIR}/efu/Graylog.cpp
  ${ESS_SOURCE_DIR}/efu/HwCheck.cpp
  ${ESS_SOURCE_DIR}/efu/Launcher.cpp
  ${ESS_SOURCE_DIR}/efu/MainProg.cpp
  ${ESS_SOURCE_DIR}/efu/Parser.cpp
  ${ESS_SOURCE_DIR}/efu/Server.cpp
  )

set(efu_common_INC
  ${VERSION_INCLUDE_DIR}/common/version_num.h
  ${VERSION_INCLUDE_DIR}/common/Version.h
  config/Config.h
  debug/Assert.h
  debug/Expect.h
  debug/Error.h
  debug/Hexdump.h
  debug/Log.h
  debug/Time.h
  debug/Trace.h
  debug/TraceGroups.h
  detector/BaseSettings.h
  detector/Detector.h
  detector/EFUArgs.h
  kafka/EV44Serializer.h
  kafka/AR51Serializer.h
  kafka/KafkaConfig.h
  kafka/Producer.h
  kafka/serializer/AbstractSerializer.h
  kafka/serializer/DA00HistogramSerializer.h
  kafka/serializer/FlatbufferTypes.h
  memory/Buffer.h
  memory/FixedSizePool.h
  memory/PoolAllocator.h
  memory/RingBuffer.h
  memory/HashMap2D.h
  math/BitMath.h
  math/NumericalMath.h
  system/gccintel.h
  system/intel.h
  system/arm.h
  system/Socket.h
  types/DetectorType.h
  JsonFile.h
  Statistics.h
  StatPublisher.h
  ${ESS_SOURCE_DIR}/efu/ExitHandler.h
  ${ESS_SOURCE_DIR}/efu/Graylog.h
  ${ESS_SOURCE_DIR}/efu/HwCheck.h
  ${ESS_SOURCE_DIR}/efu/Launcher.h
  ${ESS_SOURCE_DIR}/efu/MainProg.h
  ${ESS_SOURCE_DIR}/efu/Parser.h
  ${ESS_SOURCE_DIR}/efu/Server.h
  )

find_package(CLI11 REQUIRED)
find_package(magic_enum REQUIRED)

# Only include the 'minimum' necessary
add_library(efu_common STATIC
  ${efu_common_SRC}
  ${efu_common_INC}
  # Add required object libraries
  $<TARGET_OBJECTS:EssTimingLib>
)

# Only include the 'minimum' necessary
# aimed to be built with the UNIT_TEST macro defined
add_library(efu_common-unit_test STATIC
  ${efu_common_SRC}
  ${efu_common_INC}
  # Add required object libraries
  $<TARGET_OBJECTS:EssTimingLib>
)

# Link gtest library
target_link_libraries(efu_common-unit_test ${CONAN_LIBS_GTEST})
# Set precompiler flag UNIT_TEST for the library
target_compile_definitions(efu_common-unit_test PUBLIC -DUNIT_TEST)


# To be used as a dependency in the module's CMakeLists.txt
add_library(efu_reduction STATIC
  $<TARGET_OBJECTS:ReductionLib>
  $<TARGET_OBJECTS:ClusteringLib>
  $<TARGET_OBJECTS:MatchingLib>
  $<TARGET_OBJECTS:AnalysisLib>
  $<TARGET_OBJECTS:MonitorLib>
)

# To be used as a dependency in the module's CMakeLists.txt
# aimed to be built with the UNIT_TEST macro defined
add_library(efu_reduction-unit_test STATIC
  $<TARGET_OBJECTS:ReductionLib>
  $<TARGET_OBJECTS:ClusteringLib>
  $<TARGET_OBJECTS:MatchingLib>
  $<TARGET_OBJECTS:AnalysisLib>
  $<TARGET_OBJECTS:MonitorLib>
)

# Link gtest library
target_link_libraries(efu_reduction-unit_test ${CONAN_LIBS_GTEST})
# Set precompiler flag UNIT_TEST for the library
target_compile_definitions(efu_reduction-unit_test PUBLIC -DUNIT_TEST)

# To be used as a dependency in the module's CMakeLists.txt
add_library(efu_essreadout STATIC
  $<TARGET_OBJECTS:EssReadoutLib>
  $<TARGET_OBJECTS:EssTimingLib>
)

# To be used as a dependency in the module's CMakeLists.txt
# aimed to be built with UNIT_TEST macro defined
add_library(efu_essreadout-unit_test STATIC
  $<TARGET_OBJECTS:EssReadoutLib-unit_test>
  $<TARGET_OBJECTS:EssTimingLib-unit_test>
)

# Link gtest library
target_link_libraries(efu_essreadout-unit_test ${CONAN_LIBS_GTEST})
# Set precompiler flag UNIT_TEST for the library
target_compile_definitions(efu_essreadout-unit_test PUBLIC -DUNIT_TEST)

target_include_directories(efu_common
  PUBLIC ${CLI11_INCLUDE_DIR}
  PUBLIC ..
  )

  target_include_directories(efu_common-unit_test
  PUBLIC ${CLI11_INCLUDE_DIR}
  PUBLIC ..
  )

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(efu_common "-Wl,--no-as-needed")
endif()

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(efu_common-unit_test "-Wl,--no-as-needed")
endif()

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(efu_essreadout "-Wl,--no-as-needed")
endif()

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(efu_essreadout-unit_test "-Wl,--no-as-needed")
endif()

enable_coverage(efu_common)

target_link_libraries(efu_common ${EFU_COMMON_LIBS})


set(TestUtilsLib_INC
    testutils/SaveBuffer.h
  )
set(TestUtilsLib_SRC
    testutils/SaveBuffer.cpp
  )
add_library(TestUtilsLib OBJECT
  ${TestUtilsLib_INC}
  ${TestUtilsLib_SRC}
  )

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
