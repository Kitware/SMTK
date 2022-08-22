cmake_minimum_required(VERSION 3.12)

# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20220818: Bump to catch eigen 3.4.0 from cmb-sb.
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_item "62fec7c0f64de9b950f36a45")
  set(file_hash "8554be34f3a547701ed4aa4ef7f872980de6b4d7216e22fc13470285112bfcabcf524b51e1752f7161a8dee87775df2f389f31aa75c4e5495cb58020e2e2eaa2")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "62fec7c0f64de9b950f36a41")
  set(file_hash "c96985f574688323ffca7fc7a13e439879f961bf79962d687abcabae95c83261b89a2f4d0f9afe27e71ac97bad14f91160bf2f379127641a3d889bca0a13ac5e")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "62fec7c0f64de9b950f36a31")
  set(file_hash "138165f3f56a290a721f54456fd1e8ac048ca546048384532e3d63e0476cccff976b73409e701b74ed4fe80a1d83064c763b38337382296267fb83f733f29d33")
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
