cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2019")
  # 20201124
  set(file_item "5fc1236650a41e3d1975ab64")
  set(file_hash "1ddcce3d6e8840f573379294fb16fafde8e4f05c1090c35e66687563d94acbf3c1156108d88764b58465267bed01576e87c1581fd3b42df42154eba7eca7c396")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos")
  # 20201124
  set(file_item "5fc123aa50a41e3d1975ab9e")
  set(file_hash "88c23a4ff26860a445e706023c9fed836c45f76b0d295bded7f824172c374d21a627804776f8b68f21195760082f91238069cc55c435063b0b24186735df9471")
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
