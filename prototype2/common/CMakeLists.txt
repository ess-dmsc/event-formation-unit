cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

enable_testing()

#
# These libraries are required
#
find_package(Threads REQUIRED)
set(EFU_EXTERNAL_LIBS ${EFU_EXTERNAL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  )

find_package(LibRDKafka REQUIRED)
set(EFU_EXTERNAL_LIBS ${EFU_EXTERNAL_LIBS}
  ${LibRDKafka_LIBRARIES}
  ${LibRDKafka_C_LIBRARIES}
  )

#
# These libraries are optional
#
find_package(GraylogLogger)
if(GraylogLogger_FOUND)
  set(EFU_EXTERNAL_LIBS ${EFU_EXTERNAL_LIBS} ${GraylogLogger_LIBRARIES})
  add_definitions("-DGRAYLOG")
endif()

find_package(h5cpp)
if(h5cpp_FOUND)
  set(EFU_EXTERNAL_LIBS ${EFU_EXTERNAL_LIBS} h5cpp)
endif()


#
# Common functionality for all detector plugins
#
set(common_SRC
  NewStats.cpp
  Producer.cpp
  FBSerializer.cpp
  )
set(common_INC
  Detector.h
  NewStats.h
  Producer.h
  FBSerializer.h
  Trace.h
  EFUArgs.h
  ../../libs/include/gccintel.h
  )

option(DUMPTOFILE "Dump raw readout data to file " OFF)
if(${DUMPTOFILE})
  message(STATUS "Dumping to file enabled.")
  add_definitions("-DDUMPTOFILE")
  set(common_SRC ${common_SRC} DataSave.cpp)
  set(common_INC ${common_INC} DataSave.h)
else()
  message(STATUS "Not dumping to file.")
endif()

add_library(common STATIC
  ${common_SRC}
  ${common_INC})

enable_coverage(common)

set(EFU_COMMON_LIBS ${EFU_EXTERNAL_LIBS} common PARENT_SCOPE)
