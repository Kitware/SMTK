cmake_minimum_required(VERSION 3.12)

# Tarballs are uploaded by CI to the "ci/smtk" folder, In order to use a new superbuild, please move
# the item from the date stamped directory into the `keep` directory. This makes them available for
# download without authentication and as an indicator that the file was used at some point in CI
# itself.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20240905 - Updated to use ParaView 5.13.0
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_id "1qHh1JzfMt6KoVanF3_5oarAZbGCPHgKG")
  set(file_hash "120743938f4b95300b96b03414b9bfc2cf8c2db4e5a9844e65c22b5be8f1d43041dfeb5aa31764a3ea42566b63dc67586a1ce88a013e7ff219331a02bc60db05")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_id "1IQTuxoCbS4GQMmEGMBHlAD9tnZW0AU19")
  set(file_hash "2fc111e7acfd7c23c75699c3e64c2e3e9dadf404dfc1e84eb0b927b3a8ee7d134a3f341811135ec54de50db756f91c8c766492a8935e5d0ddf0d6f5037b759d5")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_id "1o0G2rTvNNmkzr0Hciq8dhk2unVZB6AQa")
  set(file_hash "69af98b070947d01dfb90557be10b527e7a10a7af97cc7eb68b3ace623c8bc3547b545d5a4d8707f9dcdb8f9da1a287ac6f79f0630971c637590d2225596a146")
else ()
  message(FATAL_ERROR
    "Unknown build to use for the superbuild")
endif ()

# Ensure we have a hash to verify.
if (NOT DEFINED file_id OR NOT DEFINED file_hash)
  message(FATAL_ERROR
    "Unknown file and hash for the superbuild")
endif ()

# Download the file. First download the virus scanning intertitial page form.
file(DOWNLOAD
  "https://drive.google.com/uc?export=download&id=${file_id}&authuser=0"
  ".gitlab/superbuild-gdrive.txt"
  STATUS download_status)

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
