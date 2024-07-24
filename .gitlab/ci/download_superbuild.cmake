cmake_minimum_required(VERSION 3.12)

# tarballs are uploaded by CI to the "ComputationalModelBuilder" group,
# "ci" folder, then a dated subfolder.
# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20240724: Refresh for Units Library supporting "*" units
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_item "66a13fdd5d2551c516b1d5c7")
  set(file_hash "e248699a4b7926b2bb5fc19880a460ac36c4ebe0896dab77d8232ce96fe934e539ec77ce7602d60490499b8edf756a1ad3a68c18365606e2d8b3e45c6488a240")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "66a13fdd5d2551c516b1d5c5")
  set(file_hash "807ceb6fdfad77103b849659fa033f242d8211895ebd21d9492809cae96e16fb17050daffc7f71a7fc65b8746d34c66e8d05e479fbb523b8a1b4ce0ef9af7cb5")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "66a13fdd5d2551c516b1d5c3")
  set(file_hash "a4697df02417badf76f28ab51d51db4815f13cb861a3e5f0a0d224b5dde0a389c230d385a8f03f3959b32434cadfb375ebe105c041400481d93e70711e9c8389")
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
