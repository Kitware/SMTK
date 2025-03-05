cmake_minimum_required(VERSION 3.12)

# Tarballs are uploaded by CI to the "ci/smtk" folder, In order to use a new superbuild, please move
# the item from the date stamped directory into the `keep` directory. This makes them available for
# download without authentication and as an indicator that the file was used at some point in CI
# itself.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}-{platform}.tar.gz
# 20250305 - Update to MOAB with C++17 compat
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_id "1yh4aAKx-V3vul07YlBHcjw5o3TPq4KUk")
  set(file_hash "070f1c176c7db85eed3c22d9d1dd53c1dacd1de2156120f51c17806bf59f705d60aaa1bf6bf3087019d0421b4ca1b71345f41f05ec26cbd5733d3671bd81c2e3")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_id "1uYwTX1C6pbBja_smvkGJNCOBWM-WvgFz")
  set(file_hash "4b6305631f02daf3443703d27e038b385611f0f7f89b319d2093807297a90780e232f34b1fac895ed22dc4de56a9e837e6befabb2a38c64a90321303d10b9c1f")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_id "1-TmoPhfF71TeXaI62vZ9aOHvKmeRN2t7")
  set(file_hash "dc08efb27412284c413e00c45e20cf037d906ca76a6e35021d9ba3fb26679cbdea46b8273bfbe6a448b524f78f19d70a5a5a99a8636714e7ad56f5313158a7c4")
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
