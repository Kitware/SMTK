cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20211024
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019")
  set(file_item "6173eb082fa25629b98d6351")
  set(file_hash "2ed5308344a9fa8466dcf16e1c7bb1467a44dfd70322012444b94fd675dbc2e96e47342423a1dcaff967ca241a27d99d63326208c8a16ffbcba21c1c15873f4a")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "6174f5e12fa25629b9928346")
  set(file_hash "c60ccf4db3100d76f5be6b6ddcca28ee5d4739e8e2b0caa035ec4656e4ab248dce0fb6a0335e71bb35387a81c4e850042f1810dde27b00dc5ada06b46bce4b4d")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "617515ea2fa25629b9935e3b")
  set(file_hash "37872f63ee96b2904300151ed4a52833792e4b8ce1f5f4c5beddb3d662f34e822ef4e37d8e9c9db27acbf3ea09221201eb1bf1c2b46016cce86df28713d2bdb1")
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
