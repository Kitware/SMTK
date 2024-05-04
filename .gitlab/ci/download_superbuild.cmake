cmake_minimum_required(VERSION 3.12)

# tarballs are uploaded by CI to the "ComputationalModelBuilder" group,
# "ci" folder, then a dated subfolder.
# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20240503: Refresh for new SDK usage (Xcode 14.2)
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_item "6636158ec6586ba79a5656e3")
  set(file_hash "d25862f76bbe4256498396cc5c0526ae74cdb6ed66eaf48bd9444edad4ece01f778d20db364a172abcea8a6b43573ead951052419b123d63ec1730b25e3a4ca7")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "6636158ec6586ba79a5656df")
  set(file_hash "811570d6bed9a808fd00c98ed7d435c98408034fcc2bd2705878014a88a66717af566ef549d579052a7d8582a0c7c7393164e4a22ad8abbdb2460d915de4887e")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "6636158ec6586ba79a5656dd")
  set(file_hash "cc89f4ab4b71ef1946ed9199b9d6216827cd3c9a92a1520718f448aed3a4f9afb334d516cbe06a32f9d01f1dec8232915b4baa6c42632997e37fdc5565ee9a35")
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
