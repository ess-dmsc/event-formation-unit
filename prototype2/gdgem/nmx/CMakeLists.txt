#=============================================================================
# Tests
#=============================================================================

set(NMXReadoutTest_SRC
  ReadoutTest.cpp
  )
set(NMXReadoutTest_INC
  Readout.h
  )
create_test_executable(NMXReadoutTest SKIP_MEMGRIND)

set(NMXuTPCTest_SRC
  uTPCTest.cpp
  uTPC.cpp
  )
set(NMXEventTest_INC
  uTPC.h
  )
create_test_executable(NMXuTPCTest)

set(NMXEventAnalyzerTest_SRC
  EventAnalyzerTest.cpp
  EventAnalyzer.cpp
  )
set(NMXEventAnalyzerTest_INC
  EventAnalyzer.h
  )
create_test_executable(NMXEventAnalyzerTest)

set(TrackSerializerTest_SRC
  EventAnalyzer.cpp
  TrackSerializerTest.cpp
  TrackSerializer.cpp
  uTPC.cpp
  )
set(TrackSerializerTest_INC
  EventAnalyzer.h
  TrackSerializer.h
  uTPC.h
  )
create_test_executable(TrackSerializerTest)

# Old Readout format conversion

set(srs_converter_SRC
  ${ClusteringBase_SRC}
  old_formats/srs_converter.cpp)
set(srs_converter_INC
  old_formats/OldReadoutFile.h
  ${ClusteringBase_INC})
#create_executable(srs_converter)