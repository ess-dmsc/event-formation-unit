#=============================================================================
# Common functionality for all detector plugins
#=============================================================================

add_subdirectory(monitor)
add_subdirectory(reduction)

set(efu_common_SRC
  DataSave.cpp
  DetectorModuleRegister.cpp
  EFUArgs.cpp
  EV42Serializer.cpp
  NewStats.cpp
  Producer.cpp
  Socket.cpp
  StatPublisher.cpp
  Timer.cpp
  TimeString.cpp
  TSCTimer.cpp
  )

set(efu_common_INC
  BitMath.h
  Buffer.h
  DataSave.h
  Detector.h
  DetectorModuleRegister.h
  DumpFile.h
  EFUArgs.h
  EV42Serializer.h
  gccintel.h
  Log.h
  NewStats.h
  Producer.h
  RingBuffer.h
  Socket.h
  StatPublisher.h
  TestImageUdder.h
  Timer.h
  TimeString.h
  Trace.h
  TraceGroups.h
  TSCTimer.h
  ${VERSION_INCLUDE_DIR}/common/version_num.h
  ${VERSION_INCLUDE_DIR}/common/Version.h
  )

find_package(CLI11 REQUIRED)

add_library(efu_common STATIC
  ${efu_common_SRC}
  ${efu_common_INC}
  $<TARGET_OBJECTS:ReductionObj>
  $<TARGET_OBJECTS:ClusteringObj>
  $<TARGET_OBJECTS:MatchingObj>
  $<TARGET_OBJECTS:AnalysisObj>
  $<TARGET_OBJECTS:MonitorObj>
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
