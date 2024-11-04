cmake_minimum_required(VERSION 3.12)

# Tarballs are uploaded by CI to the "ci/smtk" folder, In order to use a new superbuild, please move
# the item from the date stamped directory into the `keep` directory. This makes them available for
# download without authentication and as an indicator that the file was used at some point in CI
# itself.

set(data_host "https://data.kitware.com")

# Determine the tarball to download. ci-smtk-ci-developer-{date}-{git-sha}.tar.gz
# 20241101 - Update to use ParaView 5.13.1 and to fix Python Artifacts
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "vs2022")
  set(file_id "18XQy5PT34mDndeb_4KxiWB1Vfpa74w0F")
  set(file_hash "76141151c2d9faa7df1321b24afc7b572212ee639ffb5564ccfadc99577c280427f7444c2ca998f06c0d131f0e1272aa061f7ee2bb5923f91cde18e94792020b")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_x86_64")
  set(file_id "1T3IdfGuYvRcrH3mK1c7fzIJbsc_u7Wp7")
  set(file_hash "c9c0b1c65ec8c83620181f5a5ba54e6ed6e2d76a8ea08b800ee69c39033d4a36ca312b691eaaf86c20f3713d2a250451c231e7f6820d20bcea5d8d74235f02fc")
elseif ("$ENV{CMAKE_CONFIGURATION}" MATCHES "macos_arm64")
  set(file_id "1L9yqQMJDw_peBTDcI6AbX4s8iLiQ8XWY")
  set(file_hash "a87b1f079b8a4191d81a9b0726342714dd8ea8a60f2bc8fe9c6081fa30e2a9afaf70d3b6b821948033e01dceb7aca927b4635a69afee92f245e1f18a41c558e2")
else ()
  message(FATAL_ERROR
    "Unknown build to use for the superbuild")
endif ()

# Ensure we have a hash to verify.
if (NOT DEFINED file_id OR NOT DEFINED file_hash)
  message(FATAL_ERROR
    "Unknown file and hash for the superbuild")
endif ()

# Download the file.
file(DOWNLOAD
  "https://drive.usercontent.google.com/download?export=download&id=${file_id}&export=download&authuser=0&confirm=t"
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
