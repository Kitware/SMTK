cmake_minimum_required(VERSION 3.12)

# tarballs are uploaded by CI to the "ComputationalModelBuilder" group,
# "ci" folder, then a dated subfolder.
# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20230628: Bump to catch units dependency update.
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_item "649c62e45121da8eb1a813c2")
  set(file_hash "0685430e8ac29263e9f2d53d89eeaf60fc5e69a2c1f246665cc4aec1485ea7f42dbf1bd21dfd85809bdf4be3a8bfde2ffa1c7a9a82b4ab9cfca72381ba423f80")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "649c62e45121da8eb1a813c0")
  set(file_hash "810441065683878b8f30666b4a3d294c57528494fd5a1944983f8c110eb39ec489dae31e3b3969b7864caa591cc7be7d092150c1bdfce5d83885f66b11c34eb5")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "649c62e45121da8eb1a813be")
  set(file_hash "bec92256f6ecdee7befb40bc1ee531ad0136cc5971d72e1a60b6fabb92ae8b22b1305c28f71937b3016e96f80e2221e9263c0a0a8a4386b8b411754a0c21180f")
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
