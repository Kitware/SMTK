cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20220321: Bump to catch pybind11 superbuild change and occt for contract test.
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019")
  set(file_item "62390d504acac99f42a4ea49")
  set(file_hash "f45781726fa9583c628a23acb33f5cb7f66627a8c257c099deea2a092c79a60b45426b41491bcd1dcbe2b4c8abe5a5a0b9aeb99a09c884156ed8955fe8cd12d2")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "62390d504acac99f42a4ea45")
  set(file_hash "64bf00282ae7a572ae73d6d2257f243241cb1d362623f65d9f7ac0cf987bcd54d12ecd1944e505eafd218701c438a81407715492bc6188a4513d7c2e5cda61dd")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "62390d504acac99f42a4ea41")
  set(file_hash "6625a09e4a5885b28b683c207e301fda77d026a4c0c891653cfd561a85df9612ed8594d1abfe12057e1ccba3c868eb9ebb94c1fb5d15ff5c64a7a2c410da180f")
else ()
  message(FATAL_ERROR
    "Unknown build to use for the superbuild")
endif ()

# Ensure we have a hash to verify.
if (NOT DEFINED file_item OR NOT DEFINED file_hash)
  message(FATAL_ERROR
    "Unknown file and hash for the superbuild")
endif ()

# Download the file.
file(DOWNLOAD
  "${data_host}/api/v1/item/${file_item}/download"
  ".gitlab/superbuild.tar.gz"
  STATUS download_status
  EXPECTED_HASH "SHA512=${file_hash}")

# Check the download status.
list(GET download_status 0 res)
if (res)
  list(GET download_status 1 err)
  message(FATAL_ERROR
    "Failed to download superbuild.tar.gz: ${err}")
endif ()

# Extract the file.
execute_process(
  COMMAND
    "${CMAKE_COMMAND}"
    -E tar
    xf ".gitlab/superbuild.tar.gz"
  RESULT_VARIABLE res
  ERROR_VARIABLE err
  ERROR_STRIP_TRAILING_WHITESPACE)
if (res)
  message(FATAL_ERROR
    "Failed to extract superbuild.tar.gz: ${err}")
endif ()
