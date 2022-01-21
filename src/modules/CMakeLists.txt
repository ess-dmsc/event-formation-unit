
#=============================================================================
# Add ESS detector modules
#=============================================================================

include_directories(.)

add_subdirectory(gdgem)
add_subdirectory(freia)
add_subdirectory(multigrid)
add_subdirectory(sonde)
add_subdirectory(dream)
add_subdirectory(loki)
add_subdirectory(adc_readout)
add_subdirectory(perfgen)
add_subdirectory(cspec)

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
