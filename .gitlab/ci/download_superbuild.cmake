cmake_minimum_required(VERSION 3.12)

# tarballs are uploaded by CI to the "ComputationalModelBuilder" group,
# "ci" folder, then a dated subfolder.
# In order to use a new superbuild, please copy the item from the date stamped
# directory into the `keep` directory. This ensure that new versions will not
# be removed by the uploader script and preserve them for debugging in the
# future.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20231127: Enable Python in bundles
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_item "6564b7f7c5a2b36857ad16c4")
  set(file_hash "8b3687c140346c6b23629ff5373694cab068b5197be963ccf699b42ec708baaad80aa9d5e2eff26ee36c5eaea72629d212866c735789d1b0509c6f9756ec7a32")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_item "6564b7f7c5a2b36857ad16c2")
  set(file_hash "06513bdf597172315a27fa4dce524ea4a8588721b33f5c86833140214e3f94d2e312c80cc2d5efb4b61226d4736bb0860c377722d96f89e4a20a5c91584c1329")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_item "6564b7f6c5a2b36857ad16c0")
  set(file_hash "cb44f9cf11f18c3517828cb76f893e8265ee01a4f1a0ae89e96695c7fa5a930226a422cb9151e7845245584e85585772eca24e029846d459e934717a8b11aace")
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
