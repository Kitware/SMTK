#!/bin/sh

set -e

readonly version="3.27.4"

case "$( uname -s )" in
    Linux)
        shatool="sha256sum"
        sha256sum="186c53121cf6ef4e48b51e88690e6ef84f268611064a42e5a2e829c3d6b2efde"
        platform="linux-x86_64"
        ;;
    Darwin)
        shatool="shasum -a 256"
        sha256sum="ce15cc7ae327576a4940c816d18c75eb1b0cf7ce167604bba6b1ef6c01d03ab9"
        platform="macos-universal"
        ;;
    *)
        echo "Unrecognized platform $( uname -s )"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly platform

readonly filename="cmake-$version-$platform"
readonly tarball="$filename.tar.gz"

cd .gitlab

echo "$sha256sum  $tarball" > cmake.sha256sum
curl -OL "https://github.com/Kitware/CMake/releases/download/v$version/$tarball"
$shatool --check cmake.sha256sum
tar xf "$tarball"
mv "$filename" cmake

if [ "$( uname -s )" = "Darwin" ]; then
    ln -s CMake.app/Contents/bin cmake/bin
fi
