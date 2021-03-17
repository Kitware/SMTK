cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20210420
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019")
  set(file_item "607f0ddb2fa25629b9f66898")
  set(file_hash "c324884ec8f7832b450c948f57e6767735c88fc9a5930675891437b22b6abeb2b40fbb9cb1b498559f648693daaa71b3501e413caceccf88e714d784e9f8e75f")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos")
  set(file_item "607f0db12fa25629b9f6684f")
  set(file_hash "96f7c03cb4da0091b7ded0d680ace228b72b617471d99272d28c4e0bcd42c584a34b700ac81d1ae4a9cc4e0942bf299234a00b4f6193ff8a590b6d8f7713e39a")
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
