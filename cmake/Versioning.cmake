# Versioning.cmake
#
# set_version()
#   Set version string variable to release version or Git branch and commit.
#
# create_version_header(create_version_header)
#   Replace version string in template file and output header file in current
#   build folder. If you want to generate a new header file with an updated
#   version string, you need to run CMake again.
#
# Variables that change the behaviour of this module:
#
#   - RELEASE_VERSION: if defined, it must be a string of form "X.Y.Z[-rcW]" and
#       is be used as the version string. Otherwise, the Git branch and commit
#       are used to create the version string "<branch>-<commit>[-dirty]", where
#       "-dirty" is appended if there are non-committed chages in the Git
#       repository. When the git branch and commit are used, the version
#       component variables are all set to "0".
#
# Variables set by this module:
#
#   - VERSION_STRING: the version string defined according to the rules above.
#   - MAJOR_VERSION: the major component of the version (X)
#   - MINOR_VERSION: the minor component of the version (Y)
#   - PATCH_VERSION: the patch component of the version (Z)
#
# External requirements:
#
#   - Git (only if RELEASE_VERSION is not defined)
#
# Examples:
#
#   If RELEASE_VERSION is set to "1.2.3-rc4", VERSION_STRING will be set to
#   "1.2.3-rc4". If RELEASE_VERSION is not defined and the current Git branch is
#   "master" on commit "bc5b2c50e1fc8b1ce96607aa2f0d373900634389", with no
#   non-committed changes to tracked files, VERSION_STRING will be set to
#   "master-bc5b2c5".
#
#   # Call set_version in main CMakeLists.txt to define VERSION_STRING variable:
#   set_version()
#
#   # Generate header file substituting "@VERSION_STRING@" with the value of the
#   # variable in Version.h.in; the output file will be created in the current
#   # build directory and will be named Version.h:
#   create_version_header(Version.h.in)
#

# Set the GIT_BRANCH variable to the current Git branch.
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
    message(
      FATAL_ERROR
      "Failed to get Git branch information with error ${git_error}"
    )
  endif()
endfunction()

# Set the GIT_SHORT_REF variable to the current Git ref.
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
    message(
      FATAL_ERROR
      "Failed to get Git ref information with error ${git_error}"
    )
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

# Set VERSION_STRING if the argument has the form "X.Y.Z[-rcW]".
macro(match_and_set_version_variables version)
  if("${version}" MATCHES "^(0|[1-9][0-9]*)[.](0|[1-9][0-9]*)[.](0|[1-9][0-9]*)([-]rc[1-9][0-9]*)?$")
    set(VERSION_STRING ${version} PARENT_SCOPE)
    set(MAJOR_VERSION "${CMAKE_MATCH_1}" PARENT_SCOPE)
    set(MINOR_VERSION "${CMAKE_MATCH_2}" PARENT_SCOPE)
    set(PATCH_VERSION "${CMAKE_MATCH_3}" PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Version ${version} does not match format X.Y.Z[-rcW]")
  endif()
endmacro()

# Set VERSION_STRING to "<branch>-<commit>[-dirty]".
macro(set_version_variables_from_git_branch_and_commit)
  find_package(Git REQUIRED)
  set_git_branch_variable()
  set_git_short_ref_variable()
  set_git_dirty_variable()
  set(VERSION_STRING "${GIT_BRANCH}-${GIT_SHORT_REF}${GIT_DIRTY}" PARENT_SCOPE)
  set(MAJOR_VERSION "0" PARENT_SCOPE)
  set(MINOR_VERSION "0" PARENT_SCOPE)
  set(PATCH_VERSION "0" PARENT_SCOPE)
endmacro()

# Set version string variable to release version or Git branch and commit.
function(set_version)
  if(DEFINED RELEASE_VERSION)
    match_and_set_version_variables("${RELEASE_VERSION}")
  else()
    set_version_variables_from_git_branch_and_commit()
  endif()
endfunction()

# Replace version string in template file and output header file in current
# build folder.
macro(create_version_header version_template_file)
  # Get file name without directory or longest extension.
  get_filename_component(filename ${version_template_file} NAME_WE)
  configure_file(
    ${version_template_file}
    ${CMAKE_CURRENT_BINARY_DIR}/${filename}.h
  )
endmacro()
