#=============================================================================
# Common functionality for all detector plugins
#=============================================================================

add_subdirectory(monitor)
add_subdirectory(reduction)
add_subdirectory(readout)

set(efu_common_SRC
  DataSave.cpp
  DetectorModuleRegister.cpp
  EFUArgs.cpp
  EV42Serializer.cpp
  Hexdump.cpp
  Statistics.cpp
  Producer.cpp
  Socket.cpp
  StatPublisher.cpp
  time/Timer.cpp
  time/TimeString.cpp
  time/TSCTimer.cpp
  )

set(efu_common_INC
  Assert.h
  BitMath.h
  Buffer.h
  DataSave.h
  Detector.h
  DetectorModuleRegister.h
  DumpFile.h
  EFUArgs.h
  EV42Serializer.h
  Expect.h
  FixedSizePool.h
  Hexdump.h
  JsonFile.h
  gccintel.h
  Log.h
  Statistics.h
  PoolAllocator.h
  Producer.h
  RingBuffer.h
  Socket.h
  StatPublisher.h
  TestImageUdder.h
  time/Timer.h
  time/TimeString.h
  Trace.h
  TraceGroups.h
  time/TSCTimer.h
  ${VERSION_INCLUDE_DIR}/common/version_num.h
  ${VERSION_INCLUDE_DIR}/common/Version.h
  )

find_package(CLI11 REQUIRED)

add_library(efu_common STATIC
  ${efu_common_SRC}
  ${efu_common_INC}

  $<TARGET_OBJECTS:ReductionLib>
  $<TARGET_OBJECTS:ClusteringLib>
  $<TARGET_OBJECTS:MatchingLib>
  $<TARGET_OBJECTS:AnalysisLib>
  $<TARGET_OBJECTS:MonitorLib>
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
