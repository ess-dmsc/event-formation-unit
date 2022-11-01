
#=============================================================================
# Add ESS detector modules
#=============================================================================

include_directories(.)

set(EFUCOMMON_SRC
  ${ESS_SOURCE_DIR}/efu/ExitHandler.cpp
  ${ESS_SOURCE_DIR}/efu/HwCheck.cpp
  ${ESS_SOURCE_DIR}/efu/Launcher.cpp
  ${ESS_SOURCE_DIR}/efu/main.cpp
  ${ESS_SOURCE_DIR}/efu/Parser.cpp
  ${ESS_SOURCE_DIR}/efu/Server.cpp
  )

set(EFUCOMMON_INC
  ${ESS_SOURCE_DIR}/efu/ExitHandler.h
  ${ESS_SOURCE_DIR}/efu/HwCheck.h
  ${ESS_SOURCE_DIR}/efu/Launcher.h
  ${ESS_SOURCE_DIR}/efu/Parser.h
  ${ESS_SOURCE_DIR}/efu/Server.h
  )

add_subdirectory(miracles)
add_subdirectory(bifrost)
add_subdirectory(cspec)
add_subdirectory(dream)
add_subdirectory(freia)
add_subdirectory(loki)
add_subdirectory(perfgen)
add_subdirectory(ttlmonitor)
add_subdirectory(nmx)

#=============================================================================
# Support out-of-tree build
#=============================================================================
# ex: cmake -DEFU_EXTERNAL_DIR="/tmp/dir1;/var/dir2" ..

if(DEFINED EFU_EXTERNAL_DIR)
  foreach(builddir IN LISTS EFU_EXTERNAL_DIR)
    if(EXISTS "${builddir}")
      MESSAGE(STATUS "ECDC: Building out of source module \"${builddir}\"")
      add_subdirectory("${builddir}" "${builddir}/bin")
    else()
      MESSAGE(FATAL_ERROR "ECDC: Out of source module \"${builddir}\" doesn't exist")
    endif()
  endforeach()
endif()
