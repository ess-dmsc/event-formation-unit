include_directories(.)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(mbcaen_INC
  mbcaen/MBData.h
  mbcommon/MultiBladeEventBuilder.h
  mbcommon/DumpEventBuilderInfo.h
  )
set(mbcaen_SRC
  mbcaen.cpp
  mbcaen/MBData.cpp
  mbcommon/MultiBladeEventBuilder.cpp
  mbcommon/DumpEventBuilderInfo.cpp
  )
create_module(mbcaen)

#=============================================================================
# Build MB text 2 UDP
#=============================================================================
set(mbtext2udp_INC mbgen/MBArgs.h)
set(mbtext2udp_SRC mbtext2udp.cpp mbgen/MBArgs.cpp mbcommon/TextFile.cpp)
create_executable(mbtext2udp)

# Excecutables to build if TCLAP is installed
find_package(TCLAP)
if(TCLAP_FOUND)
  # Create an excecutable for the event-builder to process recorded data from a text-file.
  set(mbprocesstextfile_INC
    mbcommon/MultiBladeEventBuilder.h
    mbcommon/TextFile.h
    mbcommon/DumpEventBuilderInfo.h
    )
  set(mbprocesstextfile_SRC
    mbprocesstextfile.cpp
    mbcommon/MultiBladeEventBuilder.cpp
    mbcommon/TextFile.cpp
    mbcommon/DumpEventBuilderInfo.cpp
    )

  create_executable(mbprocesstextfile)

  # Create an excecutable to pass data several times for profiling
  # Not really usefull unless built with -gp
  set(mbprofiling_INC
    mbcommon/MultiBladeEventBuilder.h
    mbcommon/TextFile.h
    mbcommon/DumpEventBuilderInfo.h
    )
  set(mbprofiling_SRC mbprofiling.cpp
    mbcommon/MultiBladeEventBuilder.cpp
    mbcommon/TextFile.cpp
    mbcommon/DumpEventBuilderInfo.cpp
    )
  create_executable(mbprofiling)
else()
  message(STATUS "*** Unable to compile mbprocesstextfile as TCLAP was not found.")
  message(STATUS "*** Unable to compile mbprofiling as TCLAP was not found.")
endif()

# GOOGLE BENCHMARK
set(MBDataBenchmarkTest_INC ${mbtest_INC})
set(MBDataBenchmarkTest_SRC mbcaen/MBData.cpp mbcaen/MBDataBenchmarkTest.cpp)
create_benchmark_executable(MBDataBenchmarkTest)

# GOOGLE BENCHMARK
set(MultiBladeEventBuilderBenchmarkTest_INC ${mbtest_INC})
set(MultiBladeEventBuilderBenchmarkTest_SRC
  mbcommon/MultiBladeEventBuilder.cpp
  mbcommon/MultiBladeEventBuilderBenchmarkTest.cpp
  )
create_benchmark_executable(MultiBladeEventBuilderBenchmarkTest)


# Common include path for both tests
set(mbtest_INC mbcommon/MultiBladeEventBuilder.h mbtest/multiBladeTestData.h)

# Create excecutable for the test of counters
set(MultiBladeEventBuilderTest_INC ${mbtest_INC} )
set(MultiBladeEventBuilderTest_SRC
  mbcommon/MultiBladeEventBuilder.cpp
  mbtest/MultiBladeEventBuilderCounterTest.cpp
  )
create_test_executable(MultiBladeEventBuilderTest)

# Create excecutable for the test of clustering
set(MBClusterTest_INC ${mbtest_INC})
set(MBClusterTest_SRC
  mbcommon/MultiBladeEventBuilder.cpp
  mbtest/MultiBladeEventBuilderClusteringTest.cpp
  )
create_test_executable(MBClusterTest)

# Create excecutable for the test of geometry
set(MB16DetectorTest_INC mbcaen/MB16Detector.h)
set(MB16DetectorTest_SRC mbcaen/MB16DetectorTest.cpp)
create_test_executable(MB16DetectorTest)
