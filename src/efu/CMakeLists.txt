#=============================================================================
# Event Formation Unit
#=============================================================================
set(efu_SRC
  ExitHandler.cpp
  HwCheck.cpp
  Launcher.cpp
  main.cpp
  Parser.cpp
  Server.cpp
  ${EFU_MODULES}
  )

set(efu_INC
  ExitHandler.h
  HwCheck.h
  Launcher.h
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


add_subdirectory(test)
