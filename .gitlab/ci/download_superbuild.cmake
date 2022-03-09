cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20220309 mac, 20220307 win
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019")
  set(file_item "6227ad7c4acac99f42fb6751")
  set(file_hash "b3cba0ad747e87919e86e033869cc5e19e021ffa8b2371e076bd3d6a9d2268047447b1a11f851d2b4f31c33cfac2104227bd6f17db8be0e877b59739658d52a7")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "6228c64d4acac99f420b29a1")
  set(file_hash "43bb2399c61da42fa2db54e6af10dd9dddbfbc36e73f5ce7e015b1df7a54f9d9b35e717a5ae5b59fcfd2c36ed20436205f7209376ba1b1c5edb919139a3a67df")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "6228c64d4acac99f420b299d")
  set(file_hash "f11fef5f544abf6396c34fc3ab632d37765fc15e94b0a108d6439b13a621e328858bf1d4d3674a1c910417f95c9505a37ac4824ca3aec02258fd8797966b89b8")
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
