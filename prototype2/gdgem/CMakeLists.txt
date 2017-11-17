cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
enable_testing()
include_directories(..)
include_directories(../.. ${FLATBUFFERS_INCLUDE_DIR})

find_package(h5cpp)
set(h5_optional)

#
# Tools and Data generators
#

if (h5cpp_FOUND)
  set(gennmxfile_SRC
    nmx/Eventlet.cpp
    nmxgen/nmxfile.cpp
    nmxgen/NMXArgs.cpp
    nmxgen/ReaderVMM.cpp)
  set(gennmxfile_INC
    nmx/Eventlet.h
    nmxgen/NMXArgs.h
    nmxgen/ReaderVMM.h)
  create_executable(gennmxfile "")
  target_link_libraries(gennmxfile h5cpp_shared)
  set(h5_optional h5cpp_shared)
else ()
  message(STATUS "Unable to compile gennmxfile as h5cpp was not found.")
endif ()

#
if (PCAP_FOUND)
  include_directories(${PCAP_INCLUDE_DIR})
  set(gennmxpcap_SRC
    nmxgen/wireshark.cpp
    nmxgen/ReaderPcap.cpp
    nmxgen/NMXArgs.cpp)
  set(gennmxpcap_INC
    nmxgen/ReaderPcap.h
    nmxgen/NMXArgs.h)
  create_executable(gennmxpcap "pcap")
else ()
  message(STATUS "Unable to compile gennmxpcap as libpcap was not found.")
endif ()


#
# Detector Pipeline Plugins
#

set(gdgem_SRC
  gdgem.cpp
  NMXConfig.cpp
  ../../dataformats/multigrid/src/jsoncpp.cpp
  nmx/Clusterer.cpp
  nmx/EventNMX.cpp
  nmx/Eventlet.cpp
  nmx/Geometry.cpp
  nmx/Hists.cpp
  nmx/HistSerializer.cpp
  nmx/TrackSerializer.cpp
  nmx/AbstractEventletBuilder.cpp
  nmxgen/EventletBuilderH5.cpp
  vmm2srs/SRSMappings.cpp
  vmm2srs/SRSTime.cpp
  vmm2srs/ParserSRS.cpp
  vmm2srs/EventletBuilderSRS.cpp)
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
  nmxgen/EventletBuilderH5.h
  vmm2srs/SRSMappings.h
  vmm2srs/SRSTime.h
  vmm2srs/ParserSRS.h
  vmm2srs/EventletBuilderSRS.h)
create_module(gdgem "")
target_link_libraries(gdgem ${h5_optional})

#
# NMX / VMM Tests
#
set(HistSerializerTest_SRC
  nmx/Hists.cpp
  nmx/HistSerializerTest.cpp
  nmx/HistSerializer.cpp)
set(HistSerializerTest_INC
  nmx/Eventlet.h
  nmx/Hists.h
  nmx/HistSerializer.h
  ../common/Producer.h)
create_test_executable(HistSerializerTest "")

set(TrackSerializerTest_SRC
  nmx/Eventlet.cpp
  nmx/EventNMX.cpp
  nmx/TrackSerializerTest.cpp
  nmx/TrackSerializer.cpp)
set(TrackSerializerTest_INC
  nmx/EventNMX.h
  nmx/Eventlet.h
  nmx/TrackSerializer.h)
create_test_executable(TrackSerializerTest "")

set(ParserSRSTest_SRC
  vmm2srs/ParserSRSTest.cpp
  vmm2srs/ParserSRS.cpp
  vmm2srs/ParserSRS.h)
create_test_executable(ParserSRSTest "")

set(EventletBuilderH5Test_SRC
  ../../dataformats/multigrid/src/DataSave.cpp
  nmx/EventNMX.cpp
  nmx/Clusterer.cpp
  nmx/Hists.cpp
  nmx/AbstractEventletBuilder.cpp
  nmxgen/EventletBuilderH5Test.cpp
  nmxgen/EventletBuilderH5.cpp)
set(EventletBuilderH5Test_INC
  nmx/Clusterer.h
  nmx/EventNMX.h
  nmx/Eventlet.h
  nmx/Hists.h
  nmxgen/EventletBuilderH5.h)
create_test_executable(EventletBuilderH5Test "")
target_link_libraries(EventletBuilderH5Test ${h5_optional})

set(EventletBuilderTest_SRC
  ../../dataformats/multigrid/src/DataSave.cpp
  nmx/Clusterer.cpp
  nmx/EventNMX.cpp
  nmx/Hists.cpp
  nmx/AbstractEventletBuilder.cpp
  vmm2srs/EventletBuilderSRSTest.cpp
  vmm2srs/EventletBuilderSRS.cpp
  vmm2srs/ParserSRS.cpp
  vmm2srs/SRSMappings.cpp
  vmm2srs/SRSTime.cpp)
set(EventletBuilderTest_INC
  nmx/Clusterer.h
  nmx/EventNMX.h
  nmx/Hists.h
  nmx/Eventlet.h
  vmm2srs/ParserSRS.h
  vmm2srs/EventletBuilderSRS.h
  vmm2srs/SRSTime.h
  vmm2srs/SRSMappings.h)
create_test_executable(EventletBuilderTest "")
target_link_libraries(EventletBuilderTest ${h5_optional})

set(EventletTest_SRC
  nmx/EventletTest.cpp
  nmx/Eventlet.cpp
  nmx/Eventlet.h)
create_test_executable(EventletTest "")

set(SRSMappingsTest_SRC
  vmm2srs/SRSMappingsTest.cpp
  vmm2srs/SRSMappings.cpp)
create_test_executable(SRSMappingsTest "")

set(SRSTimeTest_SRC
  vmm2srs/SRSTimeTest.cpp
  vmm2srs/SRSTime.cpp
  vmm2srs/SRSTime.h)
create_test_executable(SRSTimeTest "")

set(GeometryTest_SRC
  nmx/GeometryTest.cpp
  nmx/Geometry.cpp
  nmx/Geometry.h)
create_test_executable(GeometryTest "")

set(ClustererTest_SRC
  nmx/ClustererTest.cpp
  nmx/Clusterer.cpp
  nmx/EventNMX.cpp)
set(ClustererTest_INC
  nmx/Clusterer.h
  nmx/EventNMX.h)
create_test_executable(ClustererTest "")

set(EventTest_SRC
  nmx/EventTest.cpp
  nmx/EventNMX.cpp
  nmx/EventNMX.h)
create_test_executable(EventTest "")
