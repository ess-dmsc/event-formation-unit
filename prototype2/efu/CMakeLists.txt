#=============================================================================
# Event Formation Unit
#=============================================================================
set(efu_SRC
  ExitHandler.cpp
  HwCheck.cpp
  Launcher.cpp
  Loader.cpp
  main.cpp
  Parser.cpp
  Server.cpp
  ${EFU_MODULES}
  )

set(efu_INC
  ExitHandler.h
  HwCheck.h
  Launcher.h
  Loader.h
  Parser.h
  Server.h
  )

create_executable(efu)

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
    )
set(ServerTest_SRC
    Server.cpp
    ServerTest.cpp
  Parser.cpp
)
create_test_executable(ServerTest)



set(ParserTestLibrary_SRC
ParserTest.cpp
)
set(ParserTestLibrary_INC
)
add_library(ParserTestLibrary STATIC
  ${ParserTestLibrary_SRC})

set(ParserTest_SRC
  Parser.cpp
)
set(ParserTest_INC
  Parser.h
)
set(ParserTest_LIB
  ParserTestLibrary)
create_test_executable(ParserTest)
