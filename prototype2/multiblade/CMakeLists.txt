include_directories(.)

#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(caen_common_inc
  MBCaenBase.h
  ../common/Hists.h
  ../common/HistSerializer.h
  caen/Readout.h
  caen/DataParser.h
  clustering/EventBuilder.h
  caen/Config.h
  )
set(caen_common_src
  MBCaenBase.cpp
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  caen/Readout.cpp
  caen/DataParser.cpp
  clustering/EventBuilder.cpp
  caen/Config.cpp
  )

set(mbcaen_INC
  ${caen_common_inc}
)
set(mbcaen_SRC
  MBCaen.cpp
  ${caen_common_src}
)
create_module(mbcaen)

set(MBReadoutTest_SRC
  caen/Readout.cpp
  caen/ReadoutTest.cpp
  )
set(MBReadoutTest_INC
  caen/Readout.h
  )
create_test_executable(MBReadoutTest SKIP_MEMGRIND)

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
    clustering/EventBuilder.h
    generators/TextFile.h
    )
  set(mbprocesstextfile_SRC
    generators/processtextfile.cpp
    generators/TextFile.cpp
    clustering/EventBuilder.cpp
    )

  create_executable(mbprocesstextfile)

  # Create an excecutable to pass data several times for profiling
  # Not really usefull unless built with -gp
  set(mbprofiling_INC
    generators/TextFile.h
    clustering/EventBuilder.h
    )
  set(mbprofiling_SRC generators/profiling.cpp
    generators/TextFile.cpp
    clustering/EventBuilder.cpp
    )
  create_executable(mbprofiling)
else()
  message(STATUS "*** Unable to compile mbprocesstextfile as TCLAP was not found.")
  message(STATUS "*** Unable to compile mbprofiling as TCLAP was not found.")
endif()

# Common include path for both tests
set(test_INC clustering/EventBuilder.h clustering/TestData.h)

# GOOGLE BENCHMARK
set(MBDataParserBenchmarkTest_INC ${test_INC})
set(MBDataParserBenchmarkTest_SRC
  caen/DataParser.cpp
  caen/DataParserBenchmarkTest.cpp)
create_benchmark_executable(MBDataParserBenchmarkTest)
if(GOOGLE_BENCHMARK)
  target_link_libraries(MBDataParserBenchmarkTest PRIVATE h5cpp)
endif()

# GOOGLE BENCHMARK
set(MBEventBuilderBenchmarkTest_INC ${test_INC})
set(MBEventBuilderBenchmarkTest_SRC
  clustering/EventBuilder.cpp
  clustering/EventBuilderBenchmarkTest.cpp
  )
create_benchmark_executable(MBEventBuilderBenchmarkTest)
if(GOOGLE_BENCHMARK)
  find_package(fmt REQUIRED)
  target_link_libraries(MBEventBuilderBenchmarkTest PRIVATE fmt::fmt)
endif()

# Create excecutable for the test of geometry
set(MBDataParserTest_INC caen/DataParser.h)
set(MBDataParserTest_SRC caen/DataParser.cpp caen/DataParserTest.cpp)
create_test_executable(MBDataParserTest)


# Create excecutable for the test of counters
set(MBEventBuilderTest_INC ${test_INC} )
set(MBEventBuilderTest_SRC
  clustering/EventBuilder.cpp
  clustering/EventBuilderCounterTest.cpp
  )
create_test_executable(MBEventBuilderTest)

# Create excecutable for the test of clustering
# set(MBClusterTest_INC ${test_INC})
# set(MBClusterTest_SRC
#   clustering/EventBuilder.cpp
#   clustering/EventBuilderClusteringTest.cpp
#   )
# create_test_executable(MBClusterTest)

#
# MJC Unit tests below
#

set(MBCAENGeometryTest_INC
  caen/MBGeometry.h
)
set(MBCAENGeometryTest_SRC
  caen/MBGeometryTest.cpp
)
create_test_executable(MBCAENGeometryTest)

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
  ../../libs/source/Timer.cpp
  ../adc_readout/test/TestUDPServer.cpp
)
create_test_executable(MBCAENBaseTest)

# Create excecutable for the test of digitizer mapping
set(DigitizerMappingTest_INC caen/DigitizerMapping.h)
set(DigitizerMappingTest_SRC caen/DigitizerMappingTest.cpp)
create_test_executable(DigitizerMappingTest)

#
set(MBConfigTest_INC caen/Config.h ../../prototype2/common/DataSave.h)
set(MBConfigTest_SRC caen/Config.cpp caen/ConfigTest.cpp ../../prototype2/common/DataSave.cpp)
create_test_executable(MBConfigTest)
