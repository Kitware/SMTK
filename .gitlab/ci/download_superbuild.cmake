cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20210420
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019")
  set(file_item "60eeffb52fa25629b96abfa2")
  set(file_hash "c5e74138f82bf540dda882e73a067fd2aa94d494f1ba8e13881484f412abccb8e8baa7948b806ed016b5754dd29bcc26b439a39ad53b03ce5cc8a1d292e3fefe")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "60eefdaf2fa25629b96abef4")
  set(file_hash "5c0d64ed1b26a39e890f7c8fab68cdfc52b1b9c3ad360dfd06de7dc9772e0a44643e093071adea5d04f45d18cf2bc21ae660c6a597d6d61c35208d0b897ca27a")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "60eeff952fa25629b96abf71")
  set(file_hash "58d95aef71a4b89061202c8ea8a4bef5e55df71527747d8b71692d1f1494b263df6edcee77c768f3f98a0e680ca461e50ddb34363ac30451c16dbf1d113bd724")
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
