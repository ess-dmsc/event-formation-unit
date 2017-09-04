cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

find_package(Threads REQUIRED)
find_package(PCAP)
find_package(LibRDKafka)
find_package(GraylogLogger)
find_package(H5CC)
find_package(HDF5 1.8.15 COMPONENTS CXX HL)
find_package(FlatBuffers REQUIRED)

set(coverage_flags "-coverage -fprofile-generate")

enable_testing()

set(DO_COV OFF)

mark_as_advanced(DO_COV)

if (${COV})
    if (${CMAKE_COMPILER_IS_GNUCXX})
        message(STATUS "Code coverage enabled.")
        set(DO_COV ON)
        set(CMAKE_BUILD_TYPE Debug)
    else()
        message(FATAL_ERROR "Unable to enable code coverage as this functionality only works with the g++ compiler.")
    endif()
endif()

include(CMakeListsUtils.txt)
add_subdirectory(gdgem)
add_subdirectory(multiblade)
add_subdirectory(multigrid)
add_subdirectory(udp)

include(CMakeListsUnitTests.txt)

add_library(calibfile OBJECT multigrid/mgcncs/CalibrationFile.cpp)

if(DO_COV)
  set_target_properties(calibfile PROPERTIES LINK_FLAGS ${coverage_flags})
  set_target_properties(calibfile PROPERTIES COMPILE_FLAGS "-g -O0 ${coverage_flags}")
endif()

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
set(common_SRC common/NewStats.cpp common/Producer.cpp common/FBSerializer.cpp
    ../dataformats/multigrid/src/DataSave.cpp)
set(common_INC common/Detector.h common/NewStats.h common/Producer.h common/FBSerializer.h
    common/Trace.h ../libs/include/gccintel.h common/EFUArgs.h)
add_library(common STATIC ${common_SRC} ${common_INC})
target_include_directories(common INTERFACE ${FLATBUFFERS_INCLUDE_DIR})

#
# the E F U
#
set(efu2_SRC common/EFUArgs.cpp efu/Loader.cpp efu/Launcher.cpp efu/main.cpp common/StatPublisher.cpp
efu/ExitHandler.cpp efu/Server.cpp efu/Parser.cpp multigrid/mgcncs/CalibrationFile.cpp)
set(efu2_INC common/EFUArgs.h efu/Launcher.h efu/Loader.h efu/Server.h efu/Parser.h common/Trace.h
    multigrid/mgcncs/CalibrationFile.h common/StatPublisher.h common/NewStats.h efu/ExitHandler.h)
set(efu_LIB ${LibRDKafka_LIBRARIES} ${CMAKE_DL_LIBS})
create_executable(efu2 "${efu_LIB}")

