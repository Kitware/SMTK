cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20211111
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019")
  set(file_item "618d799a2fa25629b9924b4f")
  set(file_hash "4cefdffd37ccc5112b82dd9a569d8427eae82a8bfec448dd3f414e66f22c650a7d6daed0cb3771f7f443e675fede7b9e1117788af53ef37be83f3cd2c91a744f")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "618d79d52fa25629b9925d79")
  set(file_hash "c865e2ec6bf667834985d8a41947186e9dc66eba0b3011b10e65e198baff1f1165bb35643d418d0cc5fa6ffb09f6801208485910b045ea0b2e4ef07fb2208af9")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "618d79ef2fa25629b9925dab")
  set(file_hash "7a3d13736a51e9f71229694417578ea7cf46a6a70e054a02e7c11cb174e355c95c036ff5a9e65dabdfa64ef862b63c15ac280b206a0fa30f939e8eb619e06492")
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
