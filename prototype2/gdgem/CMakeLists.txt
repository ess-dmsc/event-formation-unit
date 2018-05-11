#=============================================================================
# Detector Pipeline Plugins
#=============================================================================

set(gdgem_SRC
    ../../dataformats/multigrid/src/jsoncpp.cpp
    gdgem.cpp
    NMXConfig.cpp
    nmx/Clusterer.cpp
    nmx/EventNMX.cpp
    nmx/Eventlet.cpp
    nmx/Geometry.cpp
    nmx/Hists.cpp
    nmx/HistSerializer.cpp
    nmx/TrackSerializer.cpp
    nmx/AbstractEventletBuilder.cpp
    generators/EventletBuilderAPV.cpp
    vmm2srs/SRSMappings.cpp
    vmm2srs/SRSTime.cpp
    vmm2srs/ParserSRS.cpp
    vmm2srs/EventletBuilderSRS.cpp
    )
set(gdgem_INC
    NMXConfig.h
    nmx/Clusterer.h
    nmx/EventNMX.h
    nmx/Eventlet.h
    nmx/Geometry.h
    nmx/Hists.h
    nmx/HistSerializer.h
    nmx/TrackSerializer.h
    nmx/AbstractEventletBuilder.h
    generators/EventletBuilderAPV.h
    vmm2srs/SRSMappings.h
    vmm2srs/SRSTime.h
    vmm2srs/ParserSRS.h
    vmm2srs/EventletBuilderSRS.h
    )
create_module(gdgem)


#=============================================================================
# Data generators
#=============================================================================

add_subdirectory(generators)

#=============================================================================
# Tests
#=============================================================================

# Clustering

set(ClusteringBase_SRC
    dg_impl/HitsQueue.cpp
    dg_impl/HitSorter.cpp
    dg_impl/ClusterMatcher.cpp
    nmx/EventNMX.cpp
    nmx/Eventlet.cpp
    vmm2srs/SRSTime.cpp
    vmm2srs/SRSMappings.cpp)
set(ClusteringBase_INC
    dg_impl/HitContainer.h
    dg_impl/HitsQueue.h
    dg_impl/HitSorter.h
    dg_impl/AbstractClusterer.h
    dg_impl/ClusterMatcher.h
    dg_impl/TestData.h
    dg_impl/TestDataShort.h
    dg_impl/TestDataLong.h
    nmx/EventNMX.h
    nmx/Eventlet.h
    vmm2srs/SRSTime.h
    vmm2srs/SRSMappings.h)

set(HitsQueueTest_SRC
    ${ClusteringBase_SRC}
    dg_impl/HitsQueueTest.cpp)
set(HitsQueueTest_INC
    ${ClusteringBase_INC})
create_test_executable(HitsQueueTest SKIP_MEMGRIND)

set(HitSorterTest_SRC
    ${ClusteringBase_SRC}
    dg_impl/HitSorterTest.cpp)
set(HitSorterTest_INC
    ${ClusteringBase_INC})
create_test_executable(HitSorterTest SKIP_MEMGRIND)

set(Clusterer1Test_SRC
    ${ClusteringBase_SRC}
    dg_impl/Clusterer1.cpp
    dg_impl/Clusterer1Test.cpp)
set(Clusterer1Test_INC
    dg_impl/Clusterer1.h
    ${ClusteringBase_INC})
create_test_executable(Clusterer1Test SKIP_MEMGRIND)

set(ClusterMatcherTest_SRC
    ${ClusteringBase_SRC}
    dg_impl/ClusterMatcher.cpp
    dg_impl/Clusterer1.cpp
    dg_impl/ClusterMatcherTest.cpp)
set(ClusterMatcherTest_INC
    dg_impl/ClusterMatcher.h
    dg_impl/Clusterer1.h
    ${ClusteringBase_INC})
create_test_executable(ClusterMatcherTest SKIP_MEMGRIND)

# GOOGLE BENCHMARK
set(NMXClustererBenchmarkTest_SRC
    ${ClusteringBase_SRC}
    dg_impl/NMXClustererBenchmarkTest.cpp)
set(NMXClustererBenchmarkTest_INC
    ${ClusteringBase_INC})
create_benchmark_executable(NMXClustererBenchmarkTest)

set(GrayCodeBenchmarkTest_INC ../../libs/include/BitMath.h)
set(GrayCodeBenchmarkTest_SRC dg_impl/GrayCodeBenchmarkTest.cpp)
create_benchmark_executable(GrayCodeBenchmarkTest)

#
# Unit Tests
set(EventFormationTest_SRC
    nmx/AbstractEventletBuilder.cpp
    nmx/Clusterer.cpp
    nmx/EventNMX.cpp
    nmx/Hists.cpp
    vmm2srs/EventletBuilderSRS.cpp
    vmm2srs/EventFormationTest.cpp
    vmm2srs/ParserSRS.cpp
    vmm2srs/SRSTime.cpp
    vmm2srs/SRSMappings.cpp
    )
set(EventFormationTest_INC
    nmx/AbstractEventletBuilder.h
    nmx/Clusterer.h
    nmx/EventNMX.h
    nmx/Hists.h
    vmm2srs/EventletBuilderSRS.h
    vmm2srs/ParserSRS.h
    vmm2srs/SRSTime.h
    vmm2srs/SRSMappings.h
    )
create_test_executable(EventFormationTest)


set(EventletBuilderBenchmarkTest_SRC
    nmx/Clusterer.cpp
    nmx/EventNMX.cpp nmx/Hists.cpp
    nmx/AbstractEventletBuilder.cpp
    vmm2srs/EventletBuilderSRSBenchmarkTest.cpp
    vmm2srs/EventletBuilderSRS.cpp
    vmm2srs/ParserSRS.cpp
    vmm2srs/SRSMappings.cpp
    vmm2srs/SRSTime.cpp
    )
set(EventletBuilderBenchmarkTest_INC
    nmx/Clusterer.h
    nmx/EventNMX.h
    nmx/Hists.h
    nmx/Eventlet.h
    vmm2srs/ParserSRS.h
    vmm2srs/EventletBuilderSRS.h
    vmm2srs/SRSTime.h
    vmm2srs/SRSMappings.h
    )
create_benchmark_executable(EventletBuilderBenchmarkTest)


set(HistSerializerTest_SRC
    nmx/Hists.cpp
    nmx/HistSerializerTest.cpp
    nmx/HistSerializer.cpp
    )
set(HistSerializerTest_INC
    nmx/Eventlet.h
    nmx/Hists.h
    nmx/HistSerializer.h
    ../common/Producer.h
    )
create_test_executable(HistSerializerTest)

set(TrackSerializerTest_SRC
    nmx/Eventlet.cpp
    nmx/EventNMX.cpp
    nmx/TrackSerializerTest.cpp
    nmx/TrackSerializer.cpp
    )
set(TrackSerializerTest_INC
    nmx/EventNMX.h
    nmx/Eventlet.h
    nmx/TrackSerializer.h
    )
create_test_executable(TrackSerializerTest)

set(ParserSRSTest_SRC
    vmm2srs/ParserSRSTest.cpp
    vmm2srs/ParserSRS.cpp
    )
set(ParserSRSTest_INC
    vmm2srs/ParserSRS.h
    )
create_test_executable(ParserSRSTest)

set(ParserSRSVMM3Test_SRC
    vmm3srs/ParserSRSTest.cpp
    vmm3srs/ParserSRS.cpp
    )
set(ParserSRSVMM3Test_INC
    vmm3srs/ParserSRS.h
    )
create_test_executable(ParserSRSVMM3Test)

set(EventletBuilderTest_SRC
    nmx/Clusterer.cpp
    nmx/EventNMX.cpp
    nmx/Hists.cpp
    nmx/AbstractEventletBuilder.cpp
    vmm2srs/EventletBuilderSRSTest.cpp
    vmm2srs/EventletBuilderSRS.cpp
    vmm2srs/ParserSRS.cpp
    vmm2srs/SRSMappings.cpp
    vmm2srs/SRSTime.cpp
    )
set(EventletBuilderTest_INC
    nmx/Clusterer.h
    nmx/EventNMX.h
    nmx/Hists.h
    nmx/Eventlet.h
    vmm2srs/ParserSRS.h
    vmm2srs/EventletBuilderSRS.h
    vmm2srs/SRSTime.h
    vmm2srs/SRSMappings.h
    )
create_test_executable(EventletBuilderTest)

set(EventletTest_SRC
    nmx/EventletTest.cpp
    nmx/Eventlet.cpp
    )
set(EventletTest_INC
    nmx/Eventlet.h
    )
create_test_executable(EventletTest)

set(SRSMappingsTest_SRC
    vmm2srs/SRSMappingsTest.cpp
    vmm2srs/SRSMappings.cpp
    )
create_test_executable(SRSMappingsTest)

set(SRSTimeTest_SRC
    vmm2srs/SRSTimeTest.cpp
    vmm2srs/SRSTime.cpp
    vmm2srs/SRSTime.h
    )
create_test_executable(SRSTimeTest)

set(GeometryTest_SRC
    nmx/GeometryTest.cpp
    nmx/Geometry.cpp
    )
set(GeometryTest_INC
    nmx/Geometry.h
    )
create_test_executable(GeometryTest)

set(ClustererTest_SRC
    nmx/ClustererTest.cpp
    nmx/Clusterer.cpp
    nmx/EventNMX.cpp
    )
set(ClustererTest_INC
    nmx/Clusterer.h
    nmx/EventNMX.h
    )
create_test_executable(ClustererTest)

set(EventNMXTest_SRC
    nmx/EventNMXTest.cpp
    nmx/EventNMX.cpp
    )
set(EventNMXTest_INC
    nmx/EventNMX.h
    )
create_test_executable(EventNMXTest)
