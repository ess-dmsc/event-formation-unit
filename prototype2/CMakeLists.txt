cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

find_package(Threads REQUIRED)
find_package(PCAP)
find_package(LibRDKafka)
find_package(GraylogLogger)
find_package(H5CC)
find_package(HDF5 1.8.15 COMPONENTS CXX HL)

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
set(mgcncs_SRC multigrid/mgcncs.cpp multigrid/mgcncs/Data.cpp multigrid/mgcncs/ChanConv.cpp)
set(mgcncs_INC multigrid/mgcncs/Data.h multigrid/mgcncs/ChanConv.h multigrid/mgcncs/Geometry.h)
create_module(mgcncs "")


# Two-thread implementation of Multi Grid demonstrator detector
set(mgcncs2_SRC multigrid/mgcncs2.cpp multigrid/mgcncs/Data.cpp multigrid/mgcncs/ChanConv.cpp)
set(mgcncs2_INC multigrid/mgcncs/Data.h multigrid/mgcncs/ChanConv.h multigrid/mgcncs/Geometry.h)
create_module(mgcncs2 "")


#
set(nmx_SRC gdgem/gdgemh5.cpp gdgem/nmx/Clusterer.cpp gdgem/nmx/EventNMX.cpp gdgem/nmx/Eventlet.cpp gdgem/nmxgen/EventletBuilderH5.cpp)
set(nmx_INC gdgem/nmx/Clusterer.h gdgem/nmx/EventNMX.h gdgem/nmx/Eventlet.h gdgem/nmxgen/EventletBuilderH5.h)
create_module(gdgemh5 "")


# VMM2/SRS based detector (micromegas based CERN prototype)
set(nmxvmm2srs_SRC gdgem/gdgemsrs.cpp gdgem/vmm2srs/NMXVMM2SRSData.cpp gdgem/nmx/Clusterer.cpp gdgem/nmx/Geometry.cpp gdgem/nmx/Time.cpp gdgem/nmx/EventNMX.cpp gdgem/nmx/Eventlet.cpp gdgem/vmm2srs/EventletBuilder.cpp gdgem/vmm2srs/HistSerializer.cpp gdgem/vmm2srs/TrackSerializer.cpp)
set(nmxvmm2srs_INC gdgem/nmx/Clusterer.h gdgem/nmx/Geometry.h gdgem/nmx/Time.h gdgem/nmx/EventNMX.h gdgem/nmx/Eventlet.h gdgem/vmm2srs/EventletBuilder.h gdgem/vmm2srs/NMXVMM2SRSData.h gdgem/vmm2srs/HistSerializer.h gdgem/vmm2srs/TrackSerializer.h)
create_module(gdgemsrs "")


#
# the E F U
#
set(efu2_SRC common/EFUArgs.cpp efu/Loader.cpp efu/Launcher.cpp efu/main.cpp common/StatPublisher.cpp
efu/ExitHandler.cpp efu/Server.cpp efu/Parser.cpp multigrid/mgcncs/CalibrationFile.cpp)
set(efu2_INC common/EFUArgs.h efu/Launcher.h efu/Loader.h efu/Server.h efu/Parser.h common/Trace.h
    multigrid/mgcncs/CalibrationFile.h common/StatPublisher.h common/NewStats.h efu/ExitHandler.h)
set(efu_LIB ${LibRDKafka_LIBRARIES} ${CMAKE_DL_LIBS})
create_executable(efu2 "${efu_LIB}")


#
# Tools and Data generators
#

# Generates dummy 'CSPEC' data matching the Multi Grid demonstrator
set(mgcncsgen_SRC multigrid/mgcncsgen/mgcncsgen.cpp multigrid/mgcncsgen/DGArgs.cpp multigrid/mgcncs/Data.cpp)
set(mgcncsgen_INC multigrid/mgcncsgen/DGArgs.h multigrid/mgcncs/Data.h)
create_executable(mgcncsgen  "")


#
set(mgcncsgenfile_SRC multigrid/mgcncsgen/mgcncsgenfile.cpp multigrid/mgcncsgen/DGArgs.cpp)
set(mgcncsgenfile_INC multigrid/mgcncsgen/DGArgs.h)
create_executable(mgcncsgenfile "")


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


# Data generator for raw cspec data
set(mgcncsgenjson_SRC multigrid/mgcncsgen/mgcncsgenjson.cpp multigrid/mgcncsgen/JsonArgs.cpp
    ../dataformats/multigrid/src/RunSpecParse.cpp
    ../dataformats/multigrid/src/jsoncpp.cpp)
set(mgcncsgenjson_INC multigrid/mgcncsgen/JsonArgs.h)
create_executable(mgcncsgenjson "")


#
if (PCAP_FOUND)
    include_directories(${PCAP_INCLUDE_DIR})
    set(gennmxpcap_SRC gdgem/nmxgen/wireshark.cpp gdgem/nmxgen/ReaderPcap.cpp gdgem/nmxgen/NMXArgs.cpp)
    set(gennmxpcap_INC gdgem/nmxgen/ReaderPcap.h gdgem/nmxgen/NMXArgs.h)
    create_executable(gennmxpcap "pcap")
else()
    message(STATUS "Unable to compile gennmxpcap as libpcap was not found.")
endif()
