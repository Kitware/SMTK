#!/bin/sh

set -e

readonly version="3.17.3"

case "$( uname -s )" in
    Linux)
        shatool="sha256sum"
        sha256sum="da8093956f0b4ae30293c9db498da9bdeaeea4e7a2b1f2d1637ddda064d06dd0"
        platform="Linux"
        ;;
    Darwin)
        shatool="shasum -a 256"
        sha256sum="2ad1413510681b041ec305923c6ccbc64b0fed6608df82f5543577f7b4b88305"
        platform="Darwin"
        ;;
    *)
        echo "Unrecognized platform $( uname -s )"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly platform

readonly filename="cmake-$version-$platform-x86_64"
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
