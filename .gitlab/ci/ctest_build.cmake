cmake_minimum_required(VERSION 3.8)

include("${CMAKE_CURRENT_LIST_DIR}/gitlab_ci.cmake")

# Read the files from the build directory.
ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")

# Pick up from where the configure left off.
ctest_start(APPEND)

if (CTEST_CMAKE_GENERATOR STREQUAL "Unix Makefiles")
  include(ProcessorCount)
  ProcessorCount(nproc)
  set(CTEST_BUILD_FLAGS "-j${nproc}")
endif ()

ctest_build(
  NUMBER_ERRORS num_errors
  NUMBER_WARNINGS num_warnings
  RETURN_VALUE build_result)
ctest_submit(PARTS Build)

include("${CMAKE_CURRENT_LIST_DIR}/ctest_annotation.cmake")
if (DEFINED build_id)
  ctest_annotation_report("${CTEST_BINARY_DIRECTORY}/annotations.json"
    "Build Errors (${num_errors})" "https://open.cdash.org/viewBuildError.php?buildid=${build_id}"
    "Build Warnings (${num_warnings})" "https://open.cdash.org/viewBuildError.php?type=1&buildid=${build_id}"
  )
endif ()

if (build_result)
  message(FATAL_ERROR
    "Failed to build")
endif ()

if ("$ENV{CTEST_NO_WARNINGS_ALLOWED}" AND num_warnings GREATER 0)
  message(FATAL_ERROR
    "Found ${num_warnings} warnings (treating as fatal).")
endif ()
