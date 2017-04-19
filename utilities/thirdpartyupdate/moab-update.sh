#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="moab"
readonly ownership="MOAB Upstream <kwrobot@kitware.com>"
readonly subtree="thirdparty/$name"
readonly repo="https://gitlab.kitware.com/third-party/moab.git"
readonly tag="for/smtk"
readonly paths="
ANNOUNCE
AUTHORS
CMakeLists.txt
KNOWN_ISSUES
LICENSE
MOABConfig.new.cmake.in
README.md
RELEASE_NOTES
config/
configure.ac
itaps/
src/
"

extract_source () {
    git_archive
}

. "${BASH_SOURCE%/*}/update-common.sh"
