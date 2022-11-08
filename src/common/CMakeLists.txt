#=============================================================================
# Common functionality for all detector plugins
#=============================================================================

add_subdirectory(monitor)
add_subdirectory(reduction)
add_subdirectory(readout)

set(efu_common_SRC
  debug/Hexdump.cpp
  detector/EFUArgs.cpp
  kafka/EV42Serializer.cpp
  kafka/EV44Serializer.cpp
  kafka/KafkaConfig.cpp
  kafka/Producer.cpp
  system/Socket.cpp
  time/Timer.cpp
  time/TimeString.cpp
  time/TSCTimer.cpp
  Statistics.cpp
  StatPublisher.cpp
  ${ESS_SOURCE_DIR}/efu/ExitHandler.cpp
  ${ESS_SOURCE_DIR}/efu/Graylog.cpp
  ${ESS_SOURCE_DIR}/efu/HwCheck.cpp
  ${ESS_SOURCE_DIR}/efu/Launcher.cpp
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
  detector/Detector.h
  detector/EFUArgs.h
  kafka/EV42Serializer.h
  kafka/EV44Serializer.h
  kafka/KafkaConfig.h
  kafka/Producer.h
  memory/Buffer.h
  memory/FixedSizePool.h
  memory/PoolAllocator.h
  memory/RingBuffer.h
  system/gccintel.h
  system/Socket.h
  time/Timer.h
  time/TimeString.h
  time/TSCTimer.h
  BitMath.h
  DumpFile.h
  JsonFile.h
  Statistics.h
  StatPublisher.h
  ${ESS_SOURCE_DIR}/efu/ExitHandler.h
  ${ESS_SOURCE_DIR}/efu/Graylog.h
  ${ESS_SOURCE_DIR}/efu/HwCheck.h
  ${ESS_SOURCE_DIR}/efu/Launcher.h
  ${ESS_SOURCE_DIR}/efu/Parser.h
  ${ESS_SOURCE_DIR}/efu/Server.h
  )

find_package(CLI11 REQUIRED)

# Only include the 'minimum' necessary
add_library(efu_common STATIC
  ${efu_common_SRC}
  ${efu_common_INC}
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
)


target_include_directories(efu_common
  PUBLIC ${CLI11_INCLUDE_DIR}
  PUBLIC ..
  )

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(efu_common "-Wl,--no-as-needed")
endif()

enable_coverage(efu_common)

target_link_libraries(efu_common
  PUBLIC ${EFU_COMMON_LIBS}
  )

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
