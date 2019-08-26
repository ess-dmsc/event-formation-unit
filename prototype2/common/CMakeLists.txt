#=============================================================================
# Common functionality for all detector plugins
#=============================================================================

add_subdirectory(clustering)

set(efu_common_SRC
  DataSave.cpp
  DetectorModuleRegister.cpp
  DynamicHist.cpp
  EFUArgs.cpp
  EV42Serializer.cpp
  Hists.cpp
  HistSerializer.cpp
  NewStats.cpp
  Producer.cpp
  ReadoutSerializer.cpp
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
  DynamicHist.h
  EFUArgs.h
  EV42Serializer.h
  gccintel.h
  Hists.h
  HistSerializer.h
  Log.h
  NewStats.h
  Producer.h
  ReadoutSerializer.h
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
  $<TARGET_OBJECTS:ClusteringObj>)

target_include_directories(efu_common PUBLIC ${CLI11_INCLUDE_DIR} ..)

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(efu_common "-Wl,--no-as-needed")
endif()

enable_coverage(efu_common)

target_link_libraries(efu_common PUBLIC ${EFU_COMMON_LIBS})

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)
