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
  ../../common/clustering/Hit.h
  uTPC.h
  )
create_test_executable(NMXuTPCTest)



set(TrackSerializerTest_SRC
  uTPC.cpp
  TrackSerializerTest.cpp
  TrackSerializer.cpp
  )
set(TrackSerializerTest_INC
  uTPC.h
  TrackSerializer.h
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