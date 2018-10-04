include_directories(.)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(mbcaen_INC
  ../common/Hists.h
  ../common/HistSerializer.h
  mbcaen/DataParser.h
  mbcommon/MultiBladeEventBuilder.h
  mbcommon/DumpEventBuilderInfo.h
  mbcommon/MBConfig.h
  )
set(mbcaen_SRC
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  mbcaen.cpp
  mbcaen/DataParser.cpp
  mbcommon/MultiBladeEventBuilder.cpp
  mbcommon/DumpEventBuilderInfo.cpp
  mbcommon/MBConfig.cpp
  )
create_module(mbcaen)

#=============================================================================
# Build MB text 2 UDP
#=============================================================================
set(mbtext2udp_INC generators/MBArgs.h)
set(mbtext2udp_SRC generators/mbtext2udp.cpp generators/MBArgs.cpp generators/TextFile.cpp)
create_executable(mbtext2udp)

# Excecutables to build if TCLAP is installed
find_package(TCLAP)
if(TCLAP_FOUND)
  # Create an excecutable for the event-builder to process recorded data from a text-file.
  set(mbprocesstextfile_INC
    mbcommon/MultiBladeEventBuilder.h
    generators/TextFile.h
    mbcommon/DumpEventBuilderInfo.h
    )
  set(mbprocesstextfile_SRC
    generators/mbprocesstextfile.cpp
    generators/TextFile.cpp
    mbcommon/MultiBladeEventBuilder.cpp
    mbcommon/DumpEventBuilderInfo.cpp
    )

  create_executable(mbprocesstextfile)

  # Create an excecutable to pass data several times for profiling
  # Not really usefull unless built with -gp
  set(mbprofiling_INC
    generators/TextFile.h
    mbcommon/MultiBladeEventBuilder.h
    mbcommon/DumpEventBuilderInfo.h
    )
  set(mbprofiling_SRC generators/mbprofiling.cpp
    generators/TextFile.cpp
    mbcommon/MultiBladeEventBuilder.cpp
    mbcommon/DumpEventBuilderInfo.cpp
    )
  create_executable(mbprofiling)
else()
  message(STATUS "*** Unable to compile mbprocesstextfile as TCLAP was not found.")
  message(STATUS "*** Unable to compile mbprofiling as TCLAP was not found.")
endif()

# GOOGLE BENCHMARK
set(DataParserBenchmarkTest_INC ${mbtest_INC})
set(DataParserBenchmarkTest_SRC mbcaen/DataParser.cpp mbcaen/DataParserBenchmarkTest.cpp)
create_benchmark_executable(DataParserBenchmarkTest)

# GOOGLE BENCHMARK
set(MultiBladeEventBuilderBenchmarkTest_INC ${mbtest_INC})
set(MultiBladeEventBuilderBenchmarkTest_SRC
  mbcommon/MultiBladeEventBuilder.cpp
  mbcommon/MultiBladeEventBuilderBenchmarkTest.cpp
  )
create_benchmark_executable(MultiBladeEventBuilderBenchmarkTest)


# Create excecutable for the test of geometry
set(MBDataParserTest_INC mbcaen/DataParser.h)
set(MBDataParserTest_SRC mbcaen/DataParser.cpp mbcaen/DataParserTest.cpp)
create_test_executable(MBDataParserTest)


# Common include path for both tests
set(mbtest_INC mbcommon/MultiBladeEventBuilder.h mbtest/MultiBladeTestData.h)

# Create excecutable for the test of counters
set(MultiBladeEventBuilderTest_INC ${mbtest_INC} )
set(MultiBladeEventBuilderTest_SRC
  mbcommon/MultiBladeEventBuilder.cpp
  mbtest/MultiBladeEventBuilderCounterTest.cpp
  )
create_test_executable(MultiBladeEventBuilderTest)

# Create excecutable for the test of clustering
# set(MBClusterTest_INC ${mbtest_INC})
# set(MBClusterTest_SRC
#   mbcommon/MultiBladeEventBuilder.cpp
#   mbtest/MultiBladeEventBuilderClusteringTest.cpp
#   )
# create_test_executable(MBClusterTest)

#
# MJC Unit tests below
#

# Create excecutable for the test of geometry
set(MB16DetectorTest_INC mbcaen/MB16Detector.h)
set(MB16DetectorTest_SRC mbcaen/MB16DetectorTest.cpp)
create_test_executable(MB16DetectorTest)

#
set(MBConfigTest_INC mbcommon/MBConfig.h ../../prototype2/common/DataSave.h)
set(MBConfigTest_SRC mbcommon/MBConfig.cpp mbcommon/MBConfigTest.cpp ../../prototype2/common/DataSave.cpp)
create_test_executable(MBConfigTest)
