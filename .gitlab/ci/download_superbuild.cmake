cmake_minimum_required(VERSION 3.12)

# Tarballs are uploaded by CI to the "ci/smtk" folder, In order to use a new superbuild, please move
# the item from the date stamped directory into the `keep` directory. This makes them available for
# download without authentication and as an indicator that the file was used at some point in CI
# itself.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}-{platform}.tar.gz
# 20261113 - Updated to use ParaView 6.2.1
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_id "1KHiNJzgyO722S1z0IQ2kfS_dsnAC1RFs")
  set(file_hash "b7659587f0bb8cbd953b6082e86ab4b325baf0ff4d6027611a919ed2878d6d33f2b93a3d10ed7d32918db04c8335de9ad63d58a4877abc5e0eeedaa8f406927d")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_id "1vA1EGoWWGdzUGsgBDN2kFNyrBOHRRFn9")
  set(file_hash "9323f5354bb667ea97bfbf8d61680fa8da42ae7511753d09669e20434fac7feba71720ce663754e55baadf69b7f306509b856c90b55ba834b5edfcd47f4e98b4")
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
