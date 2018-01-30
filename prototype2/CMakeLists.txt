cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

enable_testing()

include_directories(. ..)

set(EFU_COMMON_LIBS)

find_package(Threads REQUIRED)
set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    )

find_package(LibRDKafka REQUIRED)
set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS}
    ${LibRDKafka_LIBRARIES}
    ${LibRDKafka_C_LIBRARIES}
    )

find_package(GraylogLogger REQUIRED)
set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS}
    ${GraylogLogger_LIBRARIES}
    )
add_definitions("-DGRAYLOG")



#
# Common functionality for all detector plugins
#
set(common_SRC
    common/NewStats.cpp
    common/Producer.cpp
    common/FBSerializer.cpp
    )
set(common_INC
    common/Detector.h
    common/NewStats.h
    common/Producer.h
    common/FBSerializer.h
    common/Trace.h
    common/EFUArgs.h
    ../libs/include/gccintel.h
    )

option(DUMPTOFILE "Dump vmm2srsdaq data to file (Doro option)" OFF)
if (${DUMPTOFILE})
  message(STATUS "Dumping to file.")
  add_definitions("-DDUMPTOFILE")
  set(common_SRC ${common_SRC} ../dataformats/multigrid/src/DataSave.cpp)
  set(common_INC ${common_INC} ../dataformats/multigrid/inc/DataSave.h)
else ()
  message(STATUS "Not dumping to file.")
endif ()

add_library(common STATIC
    ${common_SRC}
    ${common_INC})
set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} common)


include(Utilities.cmake)
add_subdirectory(gdgem)
add_subdirectory(multiblade)
add_subdirectory(multigrid)
add_subdirectory(sonde)
add_subdirectory(udp)

include(UnitTests.cmake)


#
# the E F U
#
set(efu2_SRC
    common/EFUArgs.cpp
    common/StatPublisher.cpp
    efu/Loader.cpp
    efu/Launcher.cpp
    efu/main.cpp
    efu/ExitHandler.cpp
    efu/Server.cpp
    efu/Parser.cpp
    )
set(efu2_INC
    CLI/CLI11.hpp
    common/Trace.h
    common/StatPublisher.h
    common/NewStats.h
    common/EFUArgs.h
    efu/Launcher.h
    efu/Loader.h
    efu/Server.h
    efu/Parser.h
    efu/ExitHandler.h
    )
create_executable(efu2 "${CMAKE_DL_LIBS}")
