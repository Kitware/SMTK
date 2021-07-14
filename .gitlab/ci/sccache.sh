#!/bin/sh

set -e

readonly version="0.2.15-background-init"
readonly build_date="20210602.0"

case "$( uname -s )" in
    Linux)
        shatool="sha256sum"
        sha256sum="34d62d30eae1a4145f00d62b01ad21c3456e28f11f8246c936b00cccf4855016"
        platform="x86_64-unknown-linux-musl"
        ;;
    Darwin)
        shatool="shasum -a 256"
        sha256sum="2fa396e98cc8d07e39429b187a77386db63d35409902251d462080bdd0087c22"
        platform="universal-apple-darwin"
        ;;
    *)
        echo "Unrecognized platform $( uname -s )"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly platform

readonly filename="sccache-v$version-$platform"

readonly url="https://gitlab.kitware.com/api/v4/projects/6955/packages/generic/sccache/v$version-$build_date/"

cd .gitlab

echo "$sha256sum  $filename" > sccache.sha256sum
curl -OL "$url/$filename"
$shatool --check sccache.sha256sum
mv "$filename" sccache
chmod +x sccache
