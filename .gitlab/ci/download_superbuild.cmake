cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019")
  # 20201124
  set(file_item "6033ca892fa25629b99c67d8")
  set(file_hash "55354ae4fbdfb74b1cb09e1535cb855d4bdda8370941ec7f20a6d1f2476fc145bb134d3c6d2eb6dea26168e981051237d880d4fb290a53372405fa5c4e0004f7")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos")
  # 20201124
  set(file_item "6033cb0a2fa25629b99c68b7")
  set(file_hash "9f3c39e30ca6dce10d61697d73320c08a3c999cbf28449f6b9472ad9e0fa6e4973863917cae9e1a2b201d74d92df05830607efb760455d41160f0c35cca5857a")
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
