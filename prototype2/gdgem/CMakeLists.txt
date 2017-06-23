cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
enable_testing()
include_directories(..)
include_directories(../..)
include_directories(/usr/local/include)

#
# Detector Pipeline Plugins
#
set(gdgemh5_SRC gdgemh5.cpp nmx/Clusterer.cpp nmx/EventNMX.cpp nmx/Eventlet.cpp nmxgen/EventletBuilderH5.cpp)
set(gdgemh5_INC nmx/Clusterer.h nmx/EventNMX.h nmx/Eventlet.h nmxgen/EventletBuilderH5.h)
create_module(gdgemh5 "")

# VMM2/SRS based detector (micromegas based CERN prototype)
set(gdgemsrs_SRC gdgemsrs.cpp vmm2srs/NMXVMM2SRSData.cpp nmx/Clusterer.cpp nmx/Geometry.cpp nmx/Time.cpp nmx/EventNMX.cpp nmx/Eventlet.cpp vmm2srs/EventletBuilder.cpp vmm2srs/HistSerializer.cpp vmm2srs/TrackSerializer.cpp)
set(gdgemsrs_INC nmx/Clusterer.h nmx/Geometry.h nmx/Time.h nmx/EventNMX.h nmx/Eventlet.h vmm2srs/EventletBuilder.h vmm2srs/NMXVMM2SRSData.h vmm2srs/HistSerializer.h vmm2srs/TrackSerializer.h)
create_module(gdgemsrs "")


#
# Tools and Data generators
#
if(${H5CC_FOUND} AND ${HDF5_FOUND})
    include_directories(${H5CC_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS})
    message(STATUS "Found H5CC and HDF5 libraries. Will build gennmxfile.")
    set(gennmxfile_SRC nmxgen/nmxfile.cpp nmxgen/NMXArgs.cpp nmxgen/ReaderVMM.cpp nmx/Eventlet.cpp)
    set(gennmxfile_INC nmxgen/NMXArgs.h nmxgen/ReaderVMM.h nmx/Eventlet.h)
    create_executable(gennmxfile "${H5CC_LIBRARY}")
    target_link_libraries(gennmxfile ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})
else()
    message(STATUS "Unable to find H5CC and/or HDF5 libraries. Will NOT build gennmxfile.")
endif()

#
if (PCAP_FOUND)
    include_directories(${PCAP_INCLUDE_DIR})
    set(gennmxpcap_SRC nmxgen/wireshark.cpp nmxgen/ReaderPcap.cpp nmxgen/NMXArgs.cpp)
    set(gennmxpcap_INC nmxgen/ReaderPcap.h nmxgen/NMXArgs.h)
    create_executable(gennmxpcap "pcap")
else()
    message(STATUS "Unable to compile gennmxpcap as libpcap was not found.")
endif()


#
# NMX / VMM Tests
#
set(HistSerializerTest_SRC vmm2srs/HistSerializerTest.cpp vmm2srs/HistSerializer.cpp)
set(HistSerializerTest_INC vmm2srs/HistSerializer.h ../common/Producer.h)
create_test_executable(HistSerializerTest "")

set(TrackSerializerTest_SRC vmm2srs/TrackSerializerTest.cpp vmm2srs/TrackSerializer.cpp
    nmx/Eventlet.cpp nmx/EventNMX.cpp)
set(TrackSerializerTest_INC vmm2srs/TrackSerializer.h nmx/EventNMX.h nmx/Eventlet.h)
create_test_executable(TrackSerializerTest "")

set(NMXVMM2SRSDataTest_SRC vmm2srs/NMXVMM2SRSDataTest.cpp vmm2srs/NMXVMM2SRSData.cpp
    vmm2srs/NMXVMM2SRSData.h)
create_test_executable(NMXVMM2SRSDataTest "")

set(EventletBuilderH5Test_SRC nmxgen/EventletBuilderH5Test.cpp nmxgen/EventletBuilderH5.cpp
    nmx/EventNMX.cpp nmx/Clusterer.cpp)
set(EventletBuilderH5Test_INC nmxgen/EventletBuilderH5.h nmx/Clusterer.h nmx/EventNMX.h)
create_test_executable(EventletBuilderH5Test "")

set(EventletBuilderTest_SRC vmm2srs/EventletBuilderTest.cpp vmm2srs/EventletBuilder.cpp
    vmm2srs/NMXVMM2SRSData.cpp nmx/Clusterer.cpp nmx/EventNMX.cpp nmx/Geometry.cpp nmx/Time.cpp)
set(EventletBuilderTest_INC vmm2srs/NMXVMM2SRSData.h vmm2srs/EventletBuilder.h nmx/Clusterer.h
    nmx/EventNMX.h nmx/Time.h nmx/Geometry.h)
create_test_executable(EventletBuilderTest "")

set(EventletTest_SRC nmx/EventletTest.cpp nmx/Eventlet.cpp nmx/Eventlet.h)
create_test_executable(EventletTest "")

set(GeometryTest_SRC nmx/GeometryTest.cpp nmx/Geometry.cpp)
create_test_executable(GeometryTest "")

set(TimeTest_SRC nmx/TimeTest.cpp nmx/Time.cpp nmx/Time.h)
create_test_executable(TimeTest "")

set(ClustererTest_SRC nmx/ClustererTest.cpp nmx/Clusterer.cpp nmx/EventNMX.cpp)
set(ClustererTest_INC nmx/Clusterer.h nmx/EventNMX.h)
create_test_executable(ClustererTest "")

set(EventTest_SRC nmx/EventTest.cpp nmx/EventNMX.cpp nmx/EventNMX.h)
create_test_executable(EventTest "")
