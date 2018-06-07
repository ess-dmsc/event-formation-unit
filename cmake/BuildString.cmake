function(makebuildstr)
  execute_process(COMMAND "whoami" OUTPUT_VARIABLE user_name OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND "date" "+%F %H:%M:%S" OUTPUT_VARIABLE date OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND "uname" "-n" OUTPUT_VARIABLE m_name OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND "uname" "-r" OUTPUT_VARIABLE m_version OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND "git" "rev-parse" "--short" "HEAD" OUTPUT_VARIABLE hash OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(BUILDSTR "${date} [${m_name}:${user_name}] [${m_version}] ${hash}" PARENT_SCOPE)
endfunction()

set(BUILDSTR "")
makebuildstr()

message(STATUS "Build str: ${BUILDSTR}")

add_definitions("-DBUILDSTR=${BUILDSTR}")
