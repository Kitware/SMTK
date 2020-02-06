#!/bin/sh

set -e

readonly version="3.16.4"
readonly sha256sum="12a577aa04b6639766ae908f33cf70baefc11ac4499b8b1c8812d99f05fb6a02"
readonly filename="cmake-$version-Linux-x86_64"
readonly tarball="$filename.tar.gz"

cd .gitlab

echo "$sha256sum  $tarball" > cmake.sha256sum
curl -OL "https://github.com/Kitware/CMake/releases/download/v$version/$tarball"
sha256sum --check cmake.sha256sum
tar xf "$tarball"
mv "$filename" cmake
