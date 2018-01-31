#
# These libraries are required
#
set(EFU_EXTERNAL_LIBS ${CMAKE_DL_LIBS})

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
  FBSerializer.cpp
  NewStats.cpp
  Producer.cpp
  )
set(common_INC
  ../../libs/include/gccintel.h
  Detector.h
  EFUArgs.h
  FBSerializer.h
  NewStats.h
  Producer.h
  Trace.h
  )

#
# Optionally dump to file
#
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

