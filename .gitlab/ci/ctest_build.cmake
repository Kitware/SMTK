include("${CMAKE_CURRENT_LIST_DIR}/gitlab_ci.cmake")

# Read the files from the build directory.
ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
include("${CMAKE_CURRENT_LIST_DIR}/ctest_submit_multi.cmake")

# Pick up from where the configure left off.
ctest_start(APPEND)

ctest_build(
  RETURN_VALUE build_result)
ctest_submit_multi(PARTS Build)

if (build_result)
  message(FATAL_ERROR
    "Failed to build")
endif ()
