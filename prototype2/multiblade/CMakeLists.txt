include_directories(.)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(caen_common_inc
  MBCaenBase.h
  ../common/Hists.h
  ../common/HistSerializer.h
  caen/DataParser.h
  common/EventBuilder.h
  common/DumpEventBuilderInfo.h
  common/Config.h
  )
set(caen_common_src
  MBCaenBase.cpp
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  caen/DataParser.cpp
  common/EventBuilder.cpp
  common/DumpEventBuilderInfo.cpp
  common/Config.cpp
  )

set(mbcaen_INC
  ${caen_common_inc}
)
set(mbcaen_SRC
  MBCaen.cpp
  ${caen_common_src}
)
create_module(mbcaen)


#=============================================================================
# Build MB text 2 UDP
#=============================================================================
set(mbtext2udp_INC generators/Args.h)
set(mbtext2udp_SRC generators/text2udp.cpp generators/Args.cpp generators/TextFile.cpp)
create_executable(mbtext2udp)

# Excecutables to build if TCLAP is installed
find_package(TCLAP)
if(TCLAP_FOUND)
  # Create an excecutable for the event-builder to process recorded data from a text-file.
  set(mbprocesstextfile_INC
    common/EventBuilder.h
    generators/TextFile.h
    common/DumpEventBuilderInfo.h
    )
  set(mbprocesstextfile_SRC
    generators/processtextfile.cpp
    generators/TextFile.cpp
    common/EventBuilder.cpp
    common/DumpEventBuilderInfo.cpp
    )

  create_executable(mbprocesstextfile)

  # Create an excecutable to pass data several times for profiling
  # Not really usefull unless built with -gp
  set(mbprofiling_INC
    generators/TextFile.h
    common/EventBuilder.h
    common/DumpEventBuilderInfo.h
    )
  set(mbprofiling_SRC generators/profiling.cpp
    generators/TextFile.cpp
    common/EventBuilder.cpp
    common/DumpEventBuilderInfo.cpp
    )
  create_executable(mbprofiling)
else()
  message(STATUS "*** Unable to compile mbprocesstextfile as TCLAP was not found.")
  message(STATUS "*** Unable to compile mbprofiling as TCLAP was not found.")
endif()

# GOOGLE BENCHMARK
set(MBDataParserBenchmarkTest_INC ${test_INC})
set(MBDataParserBenchmarkTest_SRC caen/DataParser.cpp caen/DataParserBenchmarkTest.cpp)
create_benchmark_executable(MBDataParserBenchmarkTest)

# GOOGLE BENCHMARK
set(EventBuilderBenchmarkTest_INC ${test_INC})
set(EventBuilderBenchmarkTest_SRC
  common/EventBuilder.cpp
  common/EventBuilderBenchmarkTest.cpp
  )
create_benchmark_executable(MBEventBuilderBenchmarkTest)


# Create excecutable for the test of geometry
set(MBDataParserTest_INC caen/DataParser.h)
set(MBDataParserTest_SRC caen/DataParser.cpp caen/DataParserTest.cpp)
create_test_executable(MBDataParserTest)


# Common include path for both tests
set(test_INC common/EventBuilder.h test/TestData.h)

# Create excecutable for the test of counters
set(MBEventBuilderTest_INC ${test_INC} )
set(MBEventBuilderTest_SRC
  common/EventBuilder.cpp
  common/DumpEventBuilderInfo.cpp
  test/EventBuilderCounterTest.cpp
  )
create_test_executable(MBEventBuilderTest)

# Create excecutable for the test of clustering
# set(MBClusterTest_INC ${test_INC})
# set(MBClusterTest_SRC
#   common/EventBuilder.cpp
#   test/EventBuilderClusteringTest.cpp
#   )
# create_test_executable(MBClusterTest)

#
# MJC Unit tests below
#

set(MBCAENBaseTest_INC
  ${caen_common_inc}
  ../../libs/include/Socket.h
  ../../libs/include/TSCTimer.h
  ../adc_readout/test/TestUDPServer.h
)
set(MBCAENBaseTest_SRC
  MBCaenBaseTest.cpp
  ${caen_common_src}
  ../../libs/source/Socket.cpp
  ../../libs/source/TSCTimer.cpp
  ../adc_readout/test/TestUDPServer.cpp
)
create_test_executable(MBCAENBaseTest)

# Create excecutable for the test of geometry
set(MB16DetectorTest_INC caen/MB16Detector.h)
set(MB16DetectorTest_SRC caen/MB16DetectorTest.cpp)
create_test_executable(MB16DetectorTest)

#
set(MBConfigTest_INC common/Config.h ../../prototype2/common/DataSave.h)
set(MBConfigTest_SRC common/Config.cpp common/ConfigTest.cpp ../../prototype2/common/DataSave.cpp)
create_test_executable(MBConfigTest)
