#=============================================================================
# Common functionality for all detector plugins
#=============================================================================
set(efu_common_SRC
  FBSerializer.cpp
  NewStats.cpp
  Producer.cpp
  )
set(efu_common_INC
  ../../libs/include/gccintel.h
  Detector.h
  EFUArgs.h
  FBSerializer.h
  NewStats.h
  Producer.h
  Trace.h
  )

if(${DUMPTOFILE})
  set(efu_common_SRC ${efu_common_SRC} DataSave.cpp)
  set(efu_common_INC ${efu_common_INC} DataSave.h)
endif()

add_library(efu_common STATIC
  ${efu_common_SRC}
  ${efu_common_INC})

enable_coverage(efu_common)

set(EFU_COMMON_LIBS ${EFU_COMMON_LIBS} efu_common PARENT_SCOPE)

