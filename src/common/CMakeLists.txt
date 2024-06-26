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
  debug/Hexdump.cpp
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
  debug/Assert.h
  debug/Expect.h
  debug/Hexdump.h
  debug/Log.h
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
  utils/EfuUtils.h
  system/gccintel.h
  system/Socket.h
  DumpFile.h
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

# Only include the 'minimum' necessary
add_library(efu_common STATIC
  ${efu_common_SRC}
  ${efu_common_INC}
  # Add required object libraries
  $<TARGET_OBJECTS:EssTimingLib>
)

# To be used as a dependency in the module's CMakeLists.txt
add_library(efu_reduction STATIC
  $<TARGET_OBJECTS:ReductionLib>
  $<TARGET_OBJECTS:ClusteringLib>
  $<TARGET_OBJECTS:MatchingLib>
  $<TARGET_OBJECTS:AnalysisLib>
  $<TARGET_OBJECTS:MonitorLib>
)

# To be used as a dependency in the module's CMakeLists.txt
add_library(efu_essreadout STATIC
  $<TARGET_OBJECTS:EssReadoutLib>
  $<TARGET_OBJECTS:EssTimingLib>
)


target_include_directories(efu_common
  PUBLIC ${CLI11_INCLUDE_DIR}
  PUBLIC ..
  )

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(efu_common "-Wl,--no-as-needed")
endif()

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(efu_essreadout "-Wl,--no-as-needed")
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
