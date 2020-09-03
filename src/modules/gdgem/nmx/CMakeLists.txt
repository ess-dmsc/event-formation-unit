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

set(NMXCombinedProcesingTest_SRC
  NMXCombinedProcesingTest.cpp
  ../generators/BuilderHits.cpp
  ../tests/HitGenerator.cpp
  )
set(NMXCombinedProcesingTest_INC
  ../generators/BuilderHits.h
  ../tests/HitGenerator.h
)
create_test_executable(NMXCombinedProcesingTest)

set(NMXBenchmarkTest_SRC
  NMXBenchmarkTest.cpp
  ../generators/BuilderHits.cpp
  ../tests/HitGenerator.cpp
)
set(NMXBenchmarkTest_INC
  ../generators/BuilderHits.h
  ../tests/HitGenerator.h
)
create_benchmark_executable(NMXBenchmarkTest)

set(TrackSerializerTest_SRC
  TrackSerializerTest.cpp
  TrackSerializer.cpp
  )
set(TrackSerializerTest_INC
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