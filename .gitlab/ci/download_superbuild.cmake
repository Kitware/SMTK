cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20230626: Bump to catch units dependency.
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_item "6499bf6e5121da8eb1a8124f")
  set(file_hash "bafc715b93bf50340cf7345ccb68e04fd2edc415bea9ddd4df30f3f749488ade4144557d39c5939bc3442c04fe3c8f919b54f11888812e5ea1d35e0aee768b24")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "6499bf6e5121da8eb1a8124d")
  set(file_hash "5dc484ea42496f98398fa90ab16c1370348f139836ac02bf82636aad0c22fefc0a106ef05c1e28daa59c4dbffc8261d1bbb5f33187dbeb6be33d94cd514c0a6d")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "6499bf6e5121da8eb1a8124b")
  set(file_hash "1c4e2b54c4ae0b36807b974c1200ffa25593a1cb462adc75746c23f7bdc42450fc201c99d6083d034876540faf11af8cc0002d7795c8565f0bd30ea1cbad74b4")
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
