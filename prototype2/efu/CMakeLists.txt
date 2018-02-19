#=============================================================================
# Event Formation Unit
#=============================================================================
set(efu_SRC
  ../common/EFUArgs.cpp
  ../common/StatPublisher.cpp
  ExitHandler.cpp
  Launcher.cpp
  Loader.cpp
  main.cpp
  Parser.cpp
  Server.cpp
  )

set(efu_INC
  ../CLI/CLI11.hpp
  ../common/StatPublisher.h
  ExitHandler.h
  Launcher.h
  Loader.h
  Parser.h
  Server.h
  )
  
add_definitions(-std=c++11)
create_executable(efu)

#=============================================================================
# EFU Tests
#=============================================================================
if(${CMAKE_COMPILER_IS_GNUCXX})
  set(ParserTestLibrary_SRC
    ParserTest.cpp
    ../test/SyscallWrappers.cpp
    ../common/NewStats.cpp)
  set(ParserTestLibrary_INC
    ../common/NewStats.h)
  add_library(ParserTestLibrary STATIC
    ${ParserTestLibrary_SRC})
  add_compile_flags(ParserTestLibrary
    "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")

  set(ParserTest_SRC
    Parser.cpp
    ../common/EFUArgs.cpp
    $<TARGET_OBJECTS:calibfile>)
  set(ParserTest_INC
    Parser.h
    ../common/EFUArgs.h)
  set(ParserTest_LIB
    ParserTestLibrary)
  create_test_executable(ParserTest)
  add_compile_flags(ParserTest
    "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
  add_linker_flags(ParserTest
    "-Wl,--wrap=fstat,--wrap=read,--wrap=write,--wrap=open")
else()
  message(STATUS "*** Skipping ParserTest - requires gcc/linux")
endif()
