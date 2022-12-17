add_executable(smtk_encode_file smtk_encode_file.cxx)

# These are added so that we can include SMTK header files
# containing compiler and system inspection macros:
target_include_directories(smtk_encode_file
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../..>
)
target_link_libraries(smtk_encode_file
  PRIVATE
    Boost::filesystem
)
smtk_install_target(smtk_encode_file)

# Copy the Boost::filesystem library into our build on windows
# so this program can be run by the build to generate header files.
if (WIN32)
  get_target_property(_bfsLibLoc Boost::filesystem IMPORTED_IMPLIB_RELEASE)
  if ("${_bfsLibLoc}" STREQUAL "")
    # If we cannot copy the boost filesystem library into the directory where
    # smtk_encode_file will appear, then smtk_encode_file will fail to run.
    # Since SMTK needs to run this utility to generate source files, we stop
    # now to make debugging easier.
    message(FATAL_ERROR "Unable to locate Boost::filesystem; smtk_encode_file will not work.")
  endif()
  string(REGEX REPLACE "lib/" "bin/" _bfsDllLoc1 "${_bfsLibLoc}")
  string(REGEX REPLACE "lib" "dll" _bfsDllLoc "${_bfsDllLoc1}")
  # We must make these directories or configure_file(…) below will create them
  # as regular files on otherwise-empty builds.
  file(MAKE_DIRECTORY "${LIBRARY_OUTPUT_PATH}")
  file(MAKE_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")
  configure_file("${_bfsLibLoc}" "${LIBRARY_OUTPUT_PATH}" COPYONLY USE_SOURCE_PERMISSIONS)
  configure_file("${_bfsDllLoc}" "${EXECUTABLE_OUTPUT_PATH}" COPYONLY USE_SOURCE_PERMISSIONS)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing/cxx)
endif()
