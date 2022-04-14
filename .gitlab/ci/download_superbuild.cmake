cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20220321: Bump to catch pybind11 superbuild change and occt for contract test.
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019")
  set(file_item "625251884acac99f42ea97d9")
  set(file_hash "b10ba9d365a611c5b73be6fc6f70b3df337ed33763fea73305649eae48ba32cd3b7affd56bf70f26ca4b74b862a6bc589eae7376d2655074af06f56a3186ca96")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "625251884acac99f42ea97d5")
  set(file_hash "a86cb87bfdb53bb719267f79338b54e60835c1c9044f455f8dddda7dfde2e12ea9c1f8ef57f50ff598465e0308e52b82906780300df6d53fe7286b0a07eb6ad2")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "625251884acac99f42ea97d1")
  set(file_hash "2ed1ebe3c4a44f660cdb3ff91dac4d9bf94981238ed4cbcb20ad41e4032fc2dfc4a0f283680875f025f887323c6f5b82b6ddf96c81ebc1471c34ae8fbca05f38")
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
