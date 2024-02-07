cmake_minimum_required(VERSION 3.8)

include("${CMAKE_CURRENT_LIST_DIR}/gitlab_ci.cmake")

# Read the files from the build directory.
ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")

# Pick up from where the configure left off.
ctest_start(APPEND)

include(ProcessorCount)
ProcessorCount(nproc)

include("${CMAKE_CURRENT_LIST_DIR}/ctest_exclusions.cmake")
ctest_test(
  PARALLEL_LEVEL "${nproc}"
  RETURN_VALUE test_result
  EXCLUDE "${test_exclusions}"
  OUTPUT_JUNIT "${CTEST_BINARY_DIRECTORY}/junit.xml")

file(GLOB packages
  "${CTEST_BINARY_DIRECTORY}/PluginTests/*/build/plugin/build/*.tar.gz")

if (NOT packages STREQUAL "")
  ctest_upload(FILES ${packages})
endif()
ctest_submit(PARTS Test)

if (test_result)
  message(FATAL_ERROR
    "Failed to test")
endif ()
