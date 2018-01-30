cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

find_package(Threads REQUIRED)
find_package(PCAP)
find_package(LibRDKafka)
find_package(GraylogLogger)

enable_testing()

include(CMakeListsUtils.txt)
add_subdirectory(gdgem)
add_subdirectory(multiblade)
add_subdirectory(multigrid)
add_subdirectory(sonde)
add_subdirectory(udp)

include(CMakeListsUnitTests.txt)

add_library(calibfile OBJECT multigrid/mgcncs/CalibrationFile.cpp)
enable_coverage_flags(calibfile)

set(PYTHON_EXECUTABLE /usr/bin/python)

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
set(common_SRC common/NewStats.cpp common/Producer.cpp common/FBSerializer.cpp)

set(common_INC common/Detector.h common/NewStats.h common/Producer.h common/FBSerializer.h
    common/Trace.h ../libs/include/gccintel.h common/EFUArgs.h)

if (DUMPTOFILE)
set(common_SRC ${common_SRC} ../dataformats/multigrid/src/DataSave.cpp)
set(common_INC ${common_INC} ../dataformats/multigrid/inc/DataSave.h)
endif ()

add_library(common STATIC ${common_SRC} ${common_INC})
target_include_directories(common INTERFACE ${FLATBUFFERS_INCLUDE_DIR})

#
# the E F U
#
set(efu2_SRC common/EFUArgs.cpp efu/Loader.cpp efu/Launcher.cpp efu/main.cpp common/StatPublisher.cpp
efu/ExitHandler.cpp efu/Server.cpp efu/Parser.cpp)
set(efu2_INC CLI/CLI11.hpp common/EFUArgs.h efu/Launcher.h efu/Loader.h efu/Server.h efu/Parser.h common/Trace.h common/StatPublisher.h common/NewStats.h efu/ExitHandler.h)
set(efu_LIB ${LibRDKafka_LIBRARIES} ${CMAKE_DL_LIBS})
create_executable(efu2 "${efu_LIB}")
