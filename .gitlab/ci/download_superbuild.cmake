cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20220902: Bump to catch ParaView widget changes.
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_item "631240d2f64de9b950e243aa")
  set(file_hash "79f324cb04bb3d850c0bfd73b4783a030e9d4cdb3524c253607bbba951c04aee12de449dbb2a0592c3e0254edf711cb6414fc6bc55ff69c11cedec799494fd2f")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "631240d2f64de9b950e243a5")
  set(file_hash "dfd4d179ebfca6e2809cc8a7d6907f66977b64fd976af8fd5c27a73ddcf5e3fa28bb7970b8da5d2ee74b37e8c1fff506259a1296722aa168a3e82831459c3c4c")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "631240d2f64de9b950e2439b")
  set(file_hash "d1541b18e0e1a000561935365e0394462af2dbd477625e130d121a3d716cbe11193021f53fe09290ffcc1466dbec596390c3c6a087f5e86efeaa30980839ff43")
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
