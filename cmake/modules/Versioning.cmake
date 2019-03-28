# Set variables from version string
#
# Usage:
#
#   set_version_variables(version)
#
# Variables defined by this module:
#
#   VERSION_STRING    The full version string, e.g. 1.2.3-rc4
#   VERSION_MAJOR     Major version number, e.g. 1 for 1.2.3-rc4
#   VERSION_MINOR     Minor version number, e.g. 2 for 1.2.3-rc4
#   VERSION_PATCH     Patch version number, e.g. 3 for 1.2.3-rc4
#   VERSION_EXTRA     Extra version information, e.g. rc4 for 1.2.3-rc4
#
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

# Set the GIT_VERSION_TAG variable if a tag is present and matches the format
# "vX.Y.Z[-rcW]". In case it does not or an error occurs, set the variable to an
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
      # A Git tag has been found; check if it has the format "vX.Y.Z[-rcW]".
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

# Check format of version string, set VERSION_STRING and break it into
# components if it has the format "X.Y.Z[-rcW]"; VERSION_EXTRA is set to an
# empty string if "-rcW" is not present. If the version string does not match
# that format, VERSION_MAJOR is set to "${version}" and the remaining component
# variables are set to empty strings.
macro(match_and_set_version_variables version)
  set(VERSION_STRING ${version})
  # Check if version has the form "X.Y.Z[-rcW]".
  if(version MATCHES "^(0|[1-9][0-9]*)[.](0|[1-9][0-9]*)[.](0|[1-9][0-9]*)([-]rc[1-9][0-9]*)?$")
    # Break the version string components into separate variables.
    set(VERSION_MAJOR "${CMAKE_MATCH_1}" PARENT_SCOPE)
    set(VERSION_MINOR "${CMAKE_MATCH_2}" PARENT_SCOPE)
    set(VERSION_PATCH "${CMAKE_MATCH_3}" PARENT_SCOPE)
    # Extra version information is optional, so it might not be available.
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

# Main module function (see module documentation).
function(set_version_variables version)
  if(GIT_FOUND)
    set_git_version_tag_variable()
    if(GIT_VERSION_TAG)
      # The version tag has the form "vX.Y.Z[-rcW]"; skip the initial 'v'.
      string(SUBSTRING "${GIT_VERSION_TAG}" 1 -1 GIT_VERSION)
      # If a version has been found in a tag, the version argument must match it.
      if(version STREQUAL GIT_VERSION)
        match_and_set_version_variables("${version}")
      else()
        message(FATAL_ERROR "${version} does not match version ${GIT_VERSION} from Git tag")
      endif()
    else()
      set_git_branch_variable()
      set_git_short_ref_variable()
      if(GIT_BRANCH STREQUAL "" OR GIT_SHORT_REF STREQUAL "")
        # Missing Git information, use provided version argument.
        match_and_set_version_variables("${version}")
      else()
        # Building from Git repo with no tag with format "vX.Y.Z[-rcW]".
        set_git_dirty_variable()
        match_and_set_version_variables("${GIT_BRANCH}-${GIT_SHORT_REF}${GIT_DIRTY}")
      endif()
    endif()
  else()
    # Git package not found by CMake, use provided version argument.
    message("Git not found for checking tags, using ${version}")
    match_and_set_version_variables("${version}")
  endif()
endfunction()
