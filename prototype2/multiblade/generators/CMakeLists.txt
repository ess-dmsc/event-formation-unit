#=============================================================================
# Build MB text 2 UDP
#=============================================================================
set(mbtext2udp_INC Args.h)
set(mbtext2udp_SRC text2udp.cpp Args.cpp TextFile.cpp)
create_executable(mbtext2udp)

# Excecutables to build if TCLAP is installed
find_package(TCLAP)
if(TCLAP_FOUND)
  # Create an excecutable for the event-builder to process recorded data from a text-file.
  set(mbprocesstextfile_INC
    ../clustering/EventBuilder.h
    TextFile.h
    )
  set(mbprocesstextfile_SRC
    processtextfile.cpp
    TextFile.cpp
    ../clustering/EventBuilder.cpp
    )

  create_executable(mbprocesstextfile)

  # Create an excecutable to pass data several times for profiling
  # Not really usefull unless built with -gp
  set(mbprofiling_INC
    TextFile.h
    ../clustering/EventBuilder.h
    )
  set(mbprofiling_SRC profiling.cpp
    TextFile.cpp
    ../clustering/EventBuilder.cpp
    )
  create_executable(mbprofiling)
else()
  message(STATUS "*** Unable to compile mbprocesstextfile as TCLAP was not found.")
  message(STATUS "*** Unable to compile mbprofiling as TCLAP was not found.")
endif()
