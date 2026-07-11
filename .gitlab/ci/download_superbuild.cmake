cmake_minimum_required(VERSION 3.12)

# Tarballs are uploaded by CI to the "ci/smtk" folder, In order to use a new superbuild, please move
# the item from the date stamped directory into the `keep` directory. This makes them available for
# download without authentication and as an indicator that the file was used at some point in CI
# itself.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}-{platform}.tar.gz
# 20260710 - Update for macOS migration to tart
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_id "19DJ7JszK1dlF3iN8qpW_eWkCKHHfSv5Q")
  set(file_hash "dee0953ec57cdda479f9388114ae14a4a4e83c0b5fe69fd46ed4cae7b3768f4a70a122d37efbcdbc6cdc20dac53a1347fc04f11cac9b4f64e393ac3f7c072d0a")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_id "17cxTIRb20ZcIRZPK9CyK5erExU2YMzom")
  set(file_hash "421199e7baecb99ffc149e6f0188b662f81995e8fa3870bcdad54d23a9dca9451388da6147369ee2d47a50e4213bc89f632c11e6ac99b2cda4d9784410c93bd2")
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
