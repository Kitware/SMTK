cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019")
  # 20200925
  set(file_item "5f73425b50a41e3d19b6e3b9")
  set(file_hash "44724f7513cef227f6ff953a0c3b023dd086bbdbe02314b602c6840594bc1703d824a4d49e1b0b586093ba016adcaf6c3cbbd4e0efac04a509f294b20a1a5dcb")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos")
  # 20200813
  set(file_item "5f36dbe19014a6d84e77387a")
  set(file_hash "d5c125e3916cf16bc194612a85ef0385532140007cb0a7ce9bda37d487d1644c3957bd6c356b4719c9b63511bb0f8c5046a4d7b35a9e353b7fd2bad2cee42114")
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
