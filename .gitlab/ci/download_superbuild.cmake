cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20220902: Bump to catch ParaView widget changes.
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_item "6493e781f04fb368544297a3")
  set(file_hash "0d7703424e0b6d3ce37f0482440886670b8898a27b4b6874ff7f33f0d85144157ffe07e92dfe9b0564d511bcb519b9905c629b3966c6b0095e17e41076bcebc6")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "6493fe19f04fb368544297e2")
  set(file_hash "92ba35489667ff5ca84d13400ac0100e422be48107dd5ebc6eb759c6ed2cbf1b21a47a72aee4a5c35d5f8e3384466a6714921ff900092ead6cc302186c848012")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "6493ea0ef04fb368544297ad")
  set(file_hash "71920f612e1e6fee33cf4f2f6f4a87302f6774fd7925f6240b74d3b0f6137053504727faf9d899293146d518f0a721b43cac1cabc4c1094cd31c9ebc883fc0dd")
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
