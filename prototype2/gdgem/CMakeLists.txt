
#
# Detector Pipeline Plugins
#
set(gdgemh5_SRC gdgem/gdgemh5.cpp gdgem/nmx/Clusterer.cpp gdgem/nmx/EventNMX.cpp gdgem/nmx/Eventlet.cpp gdgem/nmxgen/EventletBuilderH5.cpp)
set(gdgemh5_INC gdgem/nmx/Clusterer.h gdgem/nmx/EventNMX.h gdgem/nmx/Eventlet.h gdgem/nmxgen/EventletBuilderH5.h)
create_module(gdgemh5 "")

# VMM2/SRS based detector (micromegas based CERN prototype)
set(gdgemsrs_SRC gdgem/gdgemsrs.cpp gdgem/vmm2srs/NMXVMM2SRSData.cpp gdgem/nmx/Clusterer.cpp gdgem/nmx/Geometry.cpp gdgem/nmx/Time.cpp gdgem/nmx/EventNMX.cpp gdgem/nmx/Eventlet.cpp gdgem/vmm2srs/EventletBuilder.cpp gdgem/vmm2srs/HistSerializer.cpp gdgem/vmm2srs/TrackSerializer.cpp)
set(gdgemsrs_INC gdgem/nmx/Clusterer.h gdgem/nmx/Geometry.h gdgem/nmx/Time.h gdgem/nmx/EventNMX.h gdgem/nmx/Eventlet.h gdgem/vmm2srs/EventletBuilder.h gdgem/vmm2srs/NMXVMM2SRSData.h gdgem/vmm2srs/HistSerializer.h gdgem/vmm2srs/TrackSerializer.h)
create_module(gdgemsrs "")


#
# Tools and Data generators
#
if(${H5CC_FOUND} AND ${HDF5_FOUND})
    include_directories(${H5CC_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS})
    message(STATUS "Found H5CC and HDF5 libraries. Will build gennmxfile.")
    set(gennmxfile_SRC gdgem/nmxgen/nmxfile.cpp gdgem/nmxgen/NMXArgs.cpp gdgem/nmxgen/ReaderVMM.cpp gdgem/nmx/Eventlet.cpp)
    set(gennmxfile_INC gdgem/nmxgen/NMXArgs.h gdgem/nmxgen/ReaderVMM.h gdgem/nmx/Eventlet.h)
    create_executable(gennmxfile "${H5CC_LIBRARY}")
    target_link_libraries(gennmxfile ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})
else()
    message(STATUS "Unable to find H5CC and/or HDF5 libraries. Will NOT build gennmxfile.")
endif()

#
if (PCAP_FOUND)
    include_directories(${PCAP_INCLUDE_DIR})
    set(gennmxpcap_SRC gdgem/nmxgen/wireshark.cpp gdgem/nmxgen/ReaderPcap.cpp gdgem/nmxgen/NMXArgs.cpp)
    set(gennmxpcap_INC gdgem/nmxgen/ReaderPcap.h gdgem/nmxgen/NMXArgs.h)
    create_executable(gennmxpcap "pcap")
else()
    message(STATUS "Unable to compile gennmxpcap as libpcap was not found.")
endif()


#
# NMX / VMM Tests
#
set(HistSerializerTest_SRC gdgem/vmm2srs/HistSerializerTest.cpp gdgem/vmm2srs/HistSerializer.cpp)
set(HistSerializerTest_INC gdgem/vmm2srs/HistSerializer.h common/Producer.h)
create_test_executable(HistSerializerTest "")

set(TrackSerializerTest_SRC gdgem/vmm2srs/TrackSerializerTest.cpp gdgem/vmm2srs/TrackSerializer.cpp
    gdgem/nmx/Eventlet.cpp gdgem/nmx/EventNMX.cpp)
set(TrackSerializerTest_INC gdgem/vmm2srs/TrackSerializer.h gdgem/nmx/EventNMX.h gdgem/nmx/Eventlet.h)
create_test_executable(TrackSerializerTest "")

set(NMXVMM2SRSDataTest_SRC gdgem/vmm2srs/NMXVMM2SRSDataTest.cpp gdgem/vmm2srs/NMXVMM2SRSData.cpp
    gdgem/vmm2srs/NMXVMM2SRSData.h)
create_test_executable(NMXVMM2SRSDataTest "")

set(EventletBuilderH5Test_SRC gdgem/nmxgen/EventletBuilderH5Test.cpp gdgem/nmxgen/EventletBuilderH5.cpp
    gdgem/nmx/EventNMX.cpp gdgem/nmx/Clusterer.cpp)
set(EventletBuilderH5Test_INC gdgem/nmxgen/EventletBuilderH5.h gdgem/nmx/Clusterer.h gdgem/nmx/EventNMX.h)
create_test_executable(EventletBuilderH5Test "")

set(EventletBuilderTest_SRC gdgem/vmm2srs/EventletBuilderTest.cpp gdgem/vmm2srs/EventletBuilder.cpp
    gdgem/vmm2srs/NMXVMM2SRSData.cpp gdgem/nmx/Clusterer.cpp gdgem/nmx/EventNMX.cpp gdgem/nmx/Geometry.cpp gdgem/nmx/Time.cpp)
set(EventletBuilderTest_INC gdgem/vmm2srs/NMXVMM2SRSData.h gdgem/vmm2srs/EventletBuilder.h gdgem/nmx/Clusterer.h
    gdgem/nmx/EventNMX.h gdgem/nmx/Time.h gdgem/nmx/Geometry.h)
create_test_executable(EventletBuilderTest "")

set(EventletTest_SRC gdgem/nmx/EventletTest.cpp gdgem/nmx/Eventlet.cpp gdgem/nmx/Eventlet.h)
create_test_executable(EventletTest "")

set(GeometryTest_SRC gdgem/nmx/GeometryTest.cpp gdgem/nmx/Geometry.cpp)
create_test_executable(GeometryTest "")

set(TimeTest_SRC gdgem/nmx/TimeTest.cpp gdgem/nmx/Time.cpp gdgem/nmx/Time.h)
create_test_executable(TimeTest "")

set(ClustererTest_SRC gdgem/nmx/ClustererTest.cpp gdgem/nmx/Clusterer.cpp gdgem/nmx/EventNMX.cpp)
set(ClustererTest_INC gdgem/nmx/Clusterer.h gdgem/nmx/EventNMX.h)
create_test_executable(ClustererTest "")

set(EventTest_SRC gdgem/nmx/EventTest.cpp gdgem/nmx/EventNMX.cpp gdgem/nmx/EventNMX.h)
create_test_executable(EventTest "")
