
option(PROF "Enable profiling." OFF)
if(${PROF})
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pg)
  set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -pg)
  set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} -pg)
  message(STATUS "Profiling: Profiling enabled.")
else()
  message(STATUS "Profiling: Profiling disabled.")
endif()

##
## Gperftools profiline, nicked from Matthew Jones
##

find_library(GPERFTOOLS_PROFILER
        NAMES profiler
        HINTS ${Gperftools_ROOT_DIR}/lib)

find_path(GPERFTOOLS_INCLUDE_DIR
        NAMES gperftools/heap-profiler.h
        HINTS ${Gperftools_ROOT_DIR}/include)

set(GPERFTOOLS_LIBRARIES ${GPERFTOOLS_PROFILER})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
        Gperftools
        DEFAULT_MSG
        GPERFTOOLS_LIBRARIES
        GPERFTOOLS_INCLUDE_DIR)

mark_as_advanced(
        Gperftools_ROOT_DIR
        GPERFTOOLS_PROFILER
        GPERFTOOLS_LIBRARIES
        GPERFTOOLS_INCLUDE_DIR)
##

option(GPERF "Enable profiling with google perf" OFF)

if (GPERF)
  if (GPERFTOOLS_PROFILER)
    message(STATUS "Profiling: -----------------------------------------------")
    message(STATUS "Profiling: | Google perf enabled.                        |")
    message(STATUS "Profiling: | To run EFU:                                 |")
    message(STATUS "Profiling: | CPUPROFILE=/tmp/mjc.prof ./bin/efu -d modules/gdgem.so -f ../src/gdgem/configs/vmm3.json")
    message(STATUS "Profiling: | To visualise:                               |")
    message(STATUS "Profiling: | pprof --gv bin/efu /tmp/mjc.prof            |")
    message(STATUS "Profiling: -----------------------------------------------")
  else()
    message(WARNING, "Profiling: Google perf requested, but library not found")
  endif()
else()
    message(STATUS "Profiling: Google perf disabled.")
endif()
