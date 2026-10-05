cmake_minimum_required(VERSION 3.12)

# Tarballs are uploaded by CI to the "ci/smtk" folder, In order to use a new superbuild, please move
# the item from the date stamped directory into the `keep` directory. This makes them available for
# download without authentication and as an indicator that the file was used at some point in CI
# itself.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}-{platform}.tar.gz
# 20261003 - Updated to include SQL-lite support
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_id "1ln0ALJsCcZnfMFNS3sFx-ytZ5c4UduHY")
  set(file_hash "3baac3c823bd9a6c0aaa13cb12fe5ce0e2d3ad55e6b1a82890b495944fc7561663658c3e7d73dbe95e73abbdd3dc13c61fb9e9b22dc484e1b960d2b1c70052cd")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_id "1g8eJ5Qo_CY-x-mr-1FdScy3g-1oGOyUW")
  set(file_hash "fc52085e6c1056c7fb75d3b77cc3ca05e3b2138378a3687b50da3585e57bc0d2836d832b16574e65b0eec48de87f65a8cfb7a3e4a31da30dc1809240dcab4a81")
else ()
  message(FATAL_ERROR
    "Unknown build to use for the superbuild")
endif ()

# Ensure we have a hash to verify.
if (NOT DEFINED file_id OR NOT DEFINED file_hash)
  message(FATAL_ERROR
    "Unknown file and hash for the superbuild")
endif ()

# Download the file.
file(DOWNLOAD
  "https://drive.usercontent.google.com/download?export=download&id=${file_id}&export=download&authuser=0&confirm=t"
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
