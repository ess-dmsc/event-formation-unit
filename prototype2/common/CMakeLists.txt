#=============================================================================
# Common functionality for all detector plugins
#=============================================================================

set(efu_common_SRC
  clustering/AbstractMatcher.cpp
  clustering/Cluster.cpp
  clustering/GapClusterer.cpp
  clustering/Hit.cpp
  clustering/Event.cpp
  clustering/OverlapMatcher.cpp
  clustering/GapMatcher.cpp
  DynamicHist.cpp
  EV42Serializer.cpp
  EFUArgs.cpp
  NewStats.cpp
  Producer.cpp
  DataSave.cpp
  DetectorModuleRegister.cpp
  TimeString.cpp
  HistSerializer.cpp
  Hists.cpp
  ReadoutSerializer.cpp
  StatPublisher.cpp
  )

set(efu_common_INC
  clustering/AbstractClusterer.h
  clustering/AbstractMatcher.h
  clustering/Cluster.h
  clustering/GapClusterer.h
  clustering/Event.h
  clustering/Hit.h
  clustering/OverlapMatcher.h
  clustering/GapMatcher.h
  ../../libs/include/gccintel.h
  DynamicHist.h
  DetectorModuleRegister.h
  Detector.h
  EFUArgs.h
  EV42Serializer.h
  Buffer.h
  NewStats.h
  Producer.h
  Trace.h
  Log.h
  DataSave.h
  DumpFile.h
  TimeString.h
  Hists.h
  HistSerializer.h
  ReadoutSerializer.h
  RingBuffer.h
  StatPublisher.h
  )

#add_subdirectory(clustering)

find_package(CLI11 REQUIRED)

add_library(efu_common STATIC
  ${efu_common_SRC}
  ${efu_common_INC})

target_include_directories(efu_common PUBLIC ${CLI11_INCLUDE_DIR})

if(${CMAKE_COMPILER_IS_GNUCXX})
  add_linker_flags(efu_common "-Wl,--no-as-needed")
endif()

enable_coverage(efu_common)

target_link_libraries(efu_common PUBLIC ${EFU_COMMON_LIBS})

#=============================================================================
# Tests
#=============================================================================

add_subdirectory(test)

