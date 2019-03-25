find_package(Git)

# Set the GIT_BRANCH variable to the current Git branch. In case of error, set
# the variable to an empty string.
function(set_git_branch_variable)
  execute_process(
    COMMAND           "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE   git_result
    OUTPUT_VARIABLE   git_output
    ERROR_VARIABLE    git_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
  )

  if(git_result EQUAL 0)
    set(GIT_BRANCH "${git_output}" PARENT_SCOPE)
  else()
    set(GIT_BRANCH "" PARENT_SCOPE)
    message("Failed to get Git branch information")
  endif()
endfunction()

# Set the GIT_SHORT_REF variable to the current Git ref. In case of error, set
# the variable to an empty string.
function(set_git_short_ref_variable)
  execute_process(
    COMMAND           "${GIT_EXECUTABLE}" rev-parse --short HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE   git_result
    OUTPUT_VARIABLE   git_output
    ERROR_VARIABLE    git_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
  )

  if(git_result EQUAL 0)
    set(GIT_SHORT_REF "${git_output}" PARENT_SCOPE)
  else()
    set(GIT_SHORT_REF "" PARENT_SCOPE)
    message("Failed to get Git ref information")
  endif()
endfunction()

# Set the GIT_DIRTY variable to "-dirty" if there are non-committed changes to
# the Git repository. If there are no changes, the variable is set to an empty
# string.
function(set_git_dirty_variable)
  execute_process(
    COMMAND           "${GIT_EXECUTABLE}" diff-index --quiet HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE   git_result
    OUTPUT_VARIABLE   git_output
    ERROR_VARIABLE    git_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
  )

  if(git_result EQUAL 0)
    set(GIT_DIRTY "" PARENT_SCOPE)
  else()
    set(GIT_DIRTY "-dirty" PARENT_SCOPE)
  endif()
endfunction()

# Set the GIT_VERSION_TAG variable. In case of error, set the variable to an
# empty string.
function(set_git_version_tag_variable)
  execute_process(
    COMMAND           "${GIT_EXECUTABLE}" tag --list --points-at HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE   git_result
    OUTPUT_VARIABLE   git_output
    ERROR_VARIABLE    git_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
  )
  if(git_result EQUAL 0)
    if(NOT git_output STREQUAL "")
      if(git_output MATCHES "^v(0|[1-9][0-9]*)[.](0|[1-9][0-9]*)[.](0|[1-9][0-9]*)([-]rc[1-9][0-9]*)?$")
        set(GIT_VERSION_TAG "${git_output}" PARENT_SCOPE)
        message("Found Git tag ${git_output}")
      else()
        set(GIT_VERSION_TAG "" PARENT_SCOPE)
        message("Git tag ${git_output} does not match versioning format")
      endif()
    else()
      set(GIT_VERSION_TAG "" PARENT_SCOPE)
      message("No Git tag found")
    endif()
  else()
    set(GIT_VERSION_TAG "" PARENT_SCOPE)
    message("Failed to get Git tag information")
  endif()
endfunction()

macro(match_and_set_version_variables version)
  set(VERSION_STRING ${version})
  if(version MATCHES "^(0|[1-9][0-9]*)[.](0|[1-9][0-9]*)[.](0|[1-9][0-9]*)([-]rc[1-9][0-9]*)?$")
    set(VERSION_MAJOR "${CMAKE_MATCH_1}" PARENT_SCOPE)
    set(VERSION_MINOR "${CMAKE_MATCH_2}" PARENT_SCOPE)
    set(VERSION_PATCH "${CMAKE_MATCH_3}" PARENT_SCOPE)
    if(DEFINED CMAKE_MATCH_4)
      string(SUBSTRING "${CMAKE_MATCH_4}" 1 -1 VERSION_EXTRA_LOCAL)
      set(VERSION_EXTRA "${VERSION_EXTRA_LOCAL}" PARENT_SCOPE)
    else()
      set(VERSION_EXTRA "" PARENT_SCOPE)
    endif()
  else()
    set(VERSION_MAJOR "${version}" PARENT_SCOPE)
    set(VERSION_MINOR "" PARENT_SCOPE)
    set(VERSION_PATCH "" PARENT_SCOPE)
    set(VERSION_EXTRA "" PARENT_SCOPE)
  endif()
endmacro()

function(set_version_variables version)
  if(GIT_FOUND)
    set_git_version_tag_variable()
    if(GIT_VERSION_TAG)
      string(SUBSTRING "${GIT_VERSION_TAG}" 1 -1 GIT_VERSION)
      if(version STREQUAL GIT_VERSION)
        match_and_set_version_variables("${version}")
      else()
        message(FATAL_ERROR "${version} does not match version ${GIT_VERSION} from Git tag")
      endif()
    else()
      set_git_branch_variable()
      set_git_short_ref_variable()
      set_git_dirty_variable()
      if(GIT_BRANCH STREQUAL "" OR GIT_SHORT_REF STREQUAL "")
        match_and_set_version_variables("${version}")
      else()
        match_and_set_version_variables("${GIT_BRANCH}-${GIT_SHORT_REF}${GIT_DIRTY}")
      endif()
    endif()
  else()
    match_and_set_version_variables("${version}")
  endif()
endfunction()
