#=============================================================================
# Event Formation Unit
#=============================================================================
set(efu_SRC
  ../common/EFUArgs.cpp
  ../common/NewStats.cpp
  ../common/StatPublisher.cpp
  ExitHandler.cpp
  HwCheck.cpp
  Launcher.cpp
  Loader.cpp
  main.cpp
  Parser.cpp
  Server.cpp
  )

set(efu_INC
  ../common/NewStats.h
  ../common/StatPublisher.h
  ExitHandler.h
  HwCheck.h
  Launcher.h
  Loader.h
  Parser.h
  Server.h
  )


list(APPEND efu_SRC ${EFU_MODULES})
create_executable(efu)
if (NOT "${efu_LIB}" STREQUAL "")
  add_dependencies(efu ${EFU_DEPENDENCIES})
endif()

set_target_properties(efu PROPERTIES ENABLE_EXPORTS 1)
get_filename_component(EFU_RUN_INSTRUCTIONS "Executing artefacts.md" ABSOLUTE)
add_custom_command(TARGET efu POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${EFU_RUN_INSTRUCTIONS}
                "$<TARGET_FILE_DIR:efu>/")

#=============================================================================
# EFU Tests
#=============================================================================
set(ServerTest_INC
    Server.h
    Parser.h
    ../common/EFUArgs.h
    )
set(ServerTest_SRC
    Server.cpp
    ServerTest.cpp
    Parser.cpp
    ../common/EFUArgs.cpp)
create_test_executable(ServerTest)



set(ParserTestLibrary_SRC
  ParserTest.cpp
  ../common/NewStats.cpp)
set(ParserTestLibrary_INC
  ../common/NewStats.h)
add_library(ParserTestLibrary STATIC
  ${ParserTestLibrary_SRC})

set(ParserTest_SRC
  Parser.cpp
  ../common/EFUArgs.cpp)
set(ParserTest_INC
  Parser.h
  ../common/EFUArgs.h)
set(ParserTest_LIB
  ParserTestLibrary)
create_test_executable(ParserTest)
