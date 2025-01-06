cmake_minimum_required(VERSION 3.12)

# Tarballs are uploaded by CI to the "ci/smtk" folder, In order to use a new superbuild, please move
# the item from the date stamped directory into the `keep` directory. This makes them available for
# download without authentication and as an indicator that the file was used at some point in CI
# itself.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20250105 - Update to use Xcode 16.1
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_id "1fv29BZW5MedtbwV_-XuvGFNYHNFLdVxm")
  set(file_hash "7d0919c3217e143c4be6f4e7c4e47e5b9831858efc0a406f7531d09d091243fb5be44bc7ed7e26a4c61d81a5e57f38a453c253dcc8037d3ffe0fb7c9b09ba77f")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_id "1q_yl0ZXBzXZu27CqH0RJIggUO0dcdzxz")
  set(file_hash "a8cd9a32216b5c1153b1a37ff672941302004a64e066192288727c4b6be5c2fd6c57a818b9254c0d7aed5e8ed26bdd357d54224ce503468ed34b8352315f6138")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_id "1BluXPsmfRNqAqSpYahyvC6iWAVlJ0vzy")
  set(file_hash "8e8a2f8492cc383658028184a25cd555f85697e2a0075c0792321de7980eda58e7b60c0ef5be743f253d44f5abaebf89852a606a90078214bb9eed885428ce73")
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
