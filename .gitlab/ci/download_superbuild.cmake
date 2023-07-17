cmake_minimum_required(VERSION 3.12)

# tarballs are uploaded by CI to the "ComputationalModelBuilder" group,
# "ci" folder, then a dated subfolder.
# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20230717: Bump to catch units being built as a shared library.
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_item "64b54db5c887aca97227b84f")
  set(file_hash "4a2c204c473ebc406c19a3f53799c28ae2e6b66c9d321d522b2d647c4929334af2f1f97042ad07f1d7b9d9402fc1bea198d3faeafe185c5eac715b6259e6748c")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "64b54db5c887aca97227b84d")
  set(file_hash "5ba3ceefaf5bc16f7407754614d5ef00a93f3e696343dbe23a6b042b2e843a300963e9423162bafd3c00a3778e984ba9dc54a31eb427e535e181899bd999effe")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "64b54db5c887aca97227b84b")
  set(file_hash "36ce0f17564c29615d9057ff07d43c6a073fd9836d3fb2ae2466b622a76a6195da88ce4de9591d84756de9523e663c6748a2f36ef60d1ce66d682d909732764f")
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
