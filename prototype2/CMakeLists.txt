cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

find_package(Threads REQUIRED)

find_package(LibRDKafka)
if(NOT LIBRDKAFKA_FOUND)
    message(FATAL_ERROR "Unable to proceed as librdkafka could not be found.")
endif()

include_directories(. .. ${LibRDKafka_INCLUDE_DIR})

set(common_SRC common/NewStats.cpp common/Producer.cpp common/FBSerializer.cpp ../dataformats/multigrid/src/DataSave.cpp)
set(common_INC common/Detector.h common/NewStats.h common/Producer.h common/FBSerializer.h common/Trace.h ../libs/include/gccintel.h common/EFUArgs.h)

add_library(common STATIC ${common_SRC} ${common_INC})

function(create_module module_name source_files header_files link_libraries)
    add_library(${module_name} MODULE ${source_files} ${header_files})
    set_target_properties(${module_name} PROPERTIES PREFIX "")
    set_target_properties(${module_name} PROPERTIES SUFFIX ".so")
    add_dependencies(${module_name} eventlib common)
    target_link_libraries(${module_name} ${CMAKE_THREAD_LIBS_INIT} eventlib common ${LibRDKafka_LIBRARIES} ${link_libraries})
endfunction(create_module)

set(udp_so_SRC udp/udp.cpp)

set(cspec_so_SRC cspec/cspec.cpp cspec/CSPECData.cpp cspec/CSPECChanConv.cpp)
set(cspec_so_INC cspec/CSPECData.h cspec/CSPECChanConv.h common/MultiGridGeometry.h)

set(nmx_so_SRC NMX/nmx.cpp NMX/Clusterer.cpp NMX/EventNMX.cpp NMX/Eventlet.cpp nmxgen/EventletBuilderH5.cpp)
set(nmx_so_INC NMX/Clusterer.h NMX/EventNMX.h NMX/Eventlet.h nmxgen/EventletBuilderH5.h)

set(cspec2_so_SRC cspec2/cspec2.cpp cspec/CSPECData.cpp cspec/CSPECChanConv.cpp)
set(cspec2_so_INC cspec/CSPECData.h cspec/CSPECChanConv.h common/MultiGridGeometry.h)

set(nmxvmm2srs_so_SRC nmxvmm2srs/nmxvmm2srs.cpp nmxvmm2srs/NMXVMM2SRSData.cpp NMX/Clusterer.cpp NMX/Geometry.cpp NMX/Time.cpp NMX/EventNMX.cpp NMX/Eventlet.cpp nmxvmm2srs/EventletBuilder.cpp nmxvmm2srs/HistSerializer.cpp nmxvmm2srs/TrackSerializer.cpp)
set(nmxvmm2srs_so_INC NMX/Clusterer.h NMX/Geometry.h NMX/Time.h NMX/EventNMX.h NMX/Eventlet.h nmxvmm2srs/EventletBuilder.h nmxvmm2srs/NMXVMM2SRSData.h nmxvmm2srs/HistSerializer.h nmxvmm2srs/TrackSerializer.h)

create_module(cspec "${cspec_so_SRC}" "${cspec_so_INC}" "")
create_module(nmx "${nmx_so_SRC}" "${nmx_so_INC}" "")
create_module(cspec2 "${cspec2_so_SRC}" "${cspec2_so_INC}" "")
create_module(nmxvmm2srs "${nmxvmm2srs_so_SRC}" "${nmxvmm2srs_so_INC}" "")
create_module(udp "${udp_so_SRC}" "" "")

function(create_executable exec_name source_files header_files link_libraries)
    add_executable(${exec_name} ${source_files} ${header_files})
    add_dependencies(${exec_name} eventlib)
    target_link_libraries(${exec_name} ${CMAKE_THREAD_LIBS_INIT} eventlib ${LibRDKafka_LIBRARIES} ${link_libraries})
endfunction(create_executable)

set(gencspec_SRC cspecgen/cspecgen.cpp cspecgen/CspecArgs.cpp cspec/CSPECData.cpp)
set(gencspec_INC cspecgen/CspecArgs.h cspec/CSPECData.h)
create_executable(gencspec "${gencspec_SRC}" "${gencspec_INC}" "")

set(gencspecfile_SRC cspecgen/cspecfile.cpp cspecgen/CspecArgs.cpp)
set(gencspecfile_INC cspecgen/CspecArgs.h)
create_executable(gencspecfile "${gencspecfile_SRC}" "${gencspecfile_INC}" "")

set(dgmgjson_SRC mgjson/dgmgjson.cpp mgjson/MGJsonArgs.cpp ../dataformats/multigrid/src/RunSpecParse.cpp ../dataformats/multigrid/src/jsoncpp.cpp)
set(dgmgjson_INC mgjson/MGJsonArgs.h)
create_executable(dgmgjson "${dgmgjson_SRC}" "${dgmgjson_INC}" "")

set(efu_SRC common/EFUArgs.cpp efu/Loader.cpp efu/Launcher.cpp efu/main.cpp common/StatPublisher.cpp efu/ExitHandler.cpp efu/Server.cpp efu/Parser.cpp cspec/CalibrationFile.cpp)
set(efu_INC common/EFUArgs.h efu/Launcher.h efu/Loader.h efu/Server.h efu/Parser.h common/Trace.h cspec/CalibrationFile.h common/StatPublisher.h common/NewStats.h efu/ExitHandler.h)
set(efu_LIB ${LibRDKafka_LIBRARIES} ${CMAKE_DL_LIBS})
create_executable(efu2 "${efu_SRC}" "${efu_INC}" "${efu_LIB}")

find_package(PCAP)
set(gennmxpcap_SRC nmxgen/wireshark.cpp nmxgen/NMXArgs.cpp nmxgen/ReaderPcap.cpp)
set(gennmxpcap_INC nmxgen/ReaderPcap.h nmxgen/NMXArgs.h)
set(gennmxpcap_LIB ${PCAP_LIBRARY} ${DL_LIB})
if (PCAP_FOUND)
    include_directories(${PCAP_INCLUDE_DIR})
    create_executable(gennmxpcap "${gennmxpcap_SRC}" "${gennmxpcap_INC}" "${gennmxpcap_LIB}")
else()
    message(STATUS "Unable to compile gennmxpcap as libpcap was not found.")
endif()

include(CMakeListsUnitTests.txt)
