cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

find_package(Threads REQUIRED)
find_package(PCAP)
find_package(LibRDKafka)
find_package(GraylogLogger)
find_package(H5CC)
find_package(HDF5 1.8.15 COMPONENTS CXX HL REQUIRED)

set(PYTHON_EXECUTABLE /usr/bin/python)
include(CMakeListsUtils.txt)
include(CMakeListsUnitTests.txt)


if(NOT LIBRDKAFKA_FOUND)
    message(FATAL_ERROR "Unable to proceed as librdkafka could not be found.")
endif()

if(${GraylogLogger_FOUND})
    include_directories(${GraylogLogger_INCLUDE_DIR})
    add_definitions("-DGRAYLOG")
endif()

include_directories(. .. ${LibRDKafka_INCLUDE_DIR})


#
# Common functionality for all detector plugins
set(common_SRC common/NewStats.cpp common/Producer.cpp common/FBSerializer.cpp
    ../dataformats/multigrid/src/DataSave.cpp)
set(common_INC common/Detector.h common/NewStats.h common/Producer.h common/FBSerializer.h
    common/Trace.h ../libs/include/gccintel.h common/EFUArgs.h)
add_library(common STATIC ${common_SRC} ${common_INC})


# UDP receiver 'detector'
set(udp_SRC udp/udp.cpp)
create_module(udp "")
SET_TARGET_PROPERTIES(udp PROPERTIES COMPILE_FLAGS "-fno-strict-aliasing" )


# Multi Grid demonstrator detector
set(cspec_SRC cspec/cspec.cpp cspec/CSPECData.cpp cspec/CSPECChanConv.cpp)
set(cspec_INC cspec/CSPECData.h cspec/CSPECChanConv.h common/MultiGridGeometry.h)
create_module(cspec "")


# Two-thread implementation of Multi Grid demonstrator detector
set(cspec2_SRC cspec2/cspec2.cpp cspec/CSPECData.cpp cspec/CSPECChanConv.cpp)
set(cspec2_INC cspec/CSPECData.h cspec/CSPECChanConv.h common/MultiGridGeometry.h)
create_module(cspec2 "")


#
set(nmx_SRC NMX/nmx.cpp NMX/Clusterer.cpp NMX/EventNMX.cpp NMX/Eventlet.cpp nmxgen/EventletBuilderH5.cpp)
set(nmx_INC NMX/Clusterer.h NMX/EventNMX.h NMX/Eventlet.h nmxgen/EventletBuilderH5.h)
create_module(nmx "")


# VMM2/SRS based detector (micromegas based CERN prototype)
set(nmxvmm2srs_SRC nmxvmm2srs/nmxvmm2srs.cpp nmxvmm2srs/NMXVMM2SRSData.cpp NMX/Clusterer.cpp NMX/Geometry.cpp NMX/Time.cpp NMX/EventNMX.cpp NMX/Eventlet.cpp nmxvmm2srs/EventletBuilder.cpp nmxvmm2srs/HistSerializer.cpp nmxvmm2srs/TrackSerializer.cpp)
set(nmxvmm2srs_INC NMX/Clusterer.h NMX/Geometry.h NMX/Time.h NMX/EventNMX.h NMX/Eventlet.h nmxvmm2srs/EventletBuilder.h nmxvmm2srs/NMXVMM2SRSData.h nmxvmm2srs/HistSerializer.h nmxvmm2srs/TrackSerializer.h)
create_module(nmxvmm2srs "")


#
# the E F U
#
set(efu2_SRC common/EFUArgs.cpp efu/Loader.cpp efu/Launcher.cpp efu/main.cpp common/StatPublisher.cpp
    efu/ExitHandler.cpp efu/Server.cpp efu/Parser.cpp cspec/CalibrationFile.cpp)
set(efu2_INC common/EFUArgs.h efu/Launcher.h efu/Loader.h efu/Server.h efu/Parser.h common/Trace.h
    cspec/CalibrationFile.h common/StatPublisher.h common/NewStats.h efu/ExitHandler.h)
set(efu_LIB ${LibRDKafka_LIBRARIES} ${CMAKE_DL_LIBS})
create_executable(efu2 "${efu_LIB}")


#
# Tools and Data generators
#

# Generates dummy 'CSPEC' data matching the Multi Grid demonstrator
set(gencspec_SRC cspecgen/cspecgen.cpp cspecgen/CspecArgs.cpp cspec/CSPECData.cpp)
set(gencspec_INC cspecgen/CspecArgs.h cspec/CSPECData.h)
create_executable(gencspec  "")


#
set(gencspecfile_SRC cspecgen/cspecfile.cpp cspecgen/CspecArgs.cpp)
set(gencspecfile_INC cspecgen/CspecArgs.h)
create_executable(gencspecfile "")


#
if(${H5CC_FOUND} AND ${HDF5_FOUND})
    include_directories(${H5CC_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS})
    message(STATUS "Found H5CC and HDF5 libraries. Will build gennmxfile.")
    set(gennmxfile_SRC nmxgen/nmxfile.cpp nmxgen/NMXArgs.cpp nmxgen/ReaderVMM.cpp NMX/Eventlet.cpp)
    set(gennmxfile_INC nmxgen/NMXArgs.h nmxgen/ReaderVMM.h NMX/Eventlet.h)
    create_executable(gennmxfile "${H5CC_LIBRARY}")
    target_link_libraries(gennmxfile ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})
else()
    message(STATUS "Unable to find H5CC and/or HDF5 libraries. Will NOT build gennmxfile.")
endif()


# Data generator for raw cspec data
set(dgmgjson_SRC mgjson/dgmgjson.cpp mgjson/MGJsonArgs.cpp ../dataformats/multigrid/src/RunSpecParse.cpp
    ../dataformats/multigrid/src/jsoncpp.cpp)
set(dgmgjson_INC mgjson/MGJsonArgs.h)
create_executable(dgmgjson "")


#
if (PCAP_FOUND)
    include_directories(${PCAP_INCLUDE_DIR})
    set(gennmxpcap_SRC nmxgen/wireshark.cpp nmxgen/ReaderPcap.cpp nmxgen/NMXArgs.cpp)
    set(gennmxpcap_INC nmxgen/ReaderPcap.h nmxgen/NMXArgs.h)
    create_executable(gennmxpcap "pcap")
else()
    message(STATUS "Unable to compile gennmxpcap as libpcap was not found.")
endif()
