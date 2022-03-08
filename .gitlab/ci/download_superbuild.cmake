cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20220103 mac, 20220307 win
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019")
  set(file_item "6227ad7c4acac99f42fb6751")
  set(file_hash "b3cba0ad747e87919e86e033869cc5e19e021ffa8b2371e076bd3d6a9d2268047447b1a11f851d2b4f31c33cfac2104227bd6f17db8be0e877b59739658d52a7")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "61d3352a4acac99f428df8da")
  set(file_hash "a980ab964024d2bf7561beb1fa01af1fdc3d5ee07249a0e50683e2f5386163d5290a58fc76fd9fd4d319028f73b062a0129f547b7890bde07dda19045ab8ff6a")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "61d335294acac99f428df8d6")
  set(file_hash "02b115a16f72f988a97ef489507b390505e5aaa9933463d610319d5a5b73bbae48b4f0465a772be0c7d180c5808611c63db5febb52e00afbcdc382c3e1e146c6")
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
