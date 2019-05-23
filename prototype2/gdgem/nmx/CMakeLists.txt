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

set(AbstractAnalyzerTest_SRC
  AbstractAnalyzerTest.cpp
  AbstractAnalyzer.cpp
  )
set(AbstractAnalyzerTest_INC
  AbstractAnalyzer.h
  )
create_test_executable(AbstractAnalyzerTest)

set(NMXuTPCTest_SRC
  uTPCTest.cpp
  uTPC.cpp
  AbstractAnalyzer.cpp
  )
set(NMXEventTest_INC
  uTPC.h
  AbstractAnalyzer.h
  )
create_test_executable(NMXuTPCTest)

set(MGAnalysisTest_SRC
  MGAnalysisTest.cpp
  MGAnalysis.cpp
  AbstractAnalyzer.cpp
  )
set(MGAnalysisTest_INC
  MGAnalysis.h
  AbstractAnalyzer.h
  )
create_test_executable(MGAnalysisTest)

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