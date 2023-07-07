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
  set(file_item "64a81c266cb8a983de7a3613")
  set(file_hash "e26b6e2eb18e963701de43f68156e2aa3b47e812fd6eeffe0bb4f5a1e3cfafb61295b0fea47ecc4037da3aec1fd6ba039f9b13cdf802b7a494750383acc74eaa")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "64a81c266cb8a983de7a3611")
  set(file_hash "8374e7da3e6fa1f4da964164a2e43a66be8fb30e25299141ad4690bbd1985e3ba3092330d5c08dbfe54928000dd0eb5f229e59ffe11609f45a27d9712c1551c8")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "64a81c266cb8a983de7a360f")
  set(file_hash "4b7a243c73af0fc451799a229d0cbc393c38aadfd87b1f7fb7b60ee53f8f3c623a551a8d773667e971eaa5d608ef1fd3c6cdfb24e2239fdeb3de44121b8ecda3")
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
