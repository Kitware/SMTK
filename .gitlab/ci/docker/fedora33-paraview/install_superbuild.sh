#!/bin/sh

set -e
set -x

readonly superbuild_ref="$1"
shift

readonly workdir="/builds/gitlab-kitware-sciviz-ci"
git clone --recursive https://gitlab.kitware.com/cmb/cmb-superbuild.git "$workdir"
git -C "$workdir" checkout "$superbuild_ref"
git -C "$workdir" submodule update --recursive --init
export GIT_CEILING_DIRECTORIES="$workdir"
mkdir -p "$workdir/build"
cd "$workdir/build"

readonly sccache_mountpoint="/root/helpers/sccache"
sccache_settings=""
if [ -x "$sccache_mountpoint" ]; then
    sccache_settings="-DCMAKE_C_COMPILER_LAUNCHER=$sccache_mountpoint -DCMAKE_CXX_COMPILER_LAUNCHER=$sccache_mountpoint"
    "$sccache_mountpoint" --start-server
    "$sccache_mountpoint" --show-stats
fi
readonly sccache_settings

cmake -GNinja \
    -DDEVELOPER_MODE_smtk:BOOL=ON \
    -DENABLE_cmb:BOOL=OFF \
    -DENABLE_cmbusersguide:BOOL=OFF \
    -DENABLE_occt:BOOL=ON \
    -DUSE_SYSTEM_fontconfig:BOOL=ON \
    -DENABLE_paraview:BOOL=ON \
    -DENABLE_python3:BOOL=ON \
    -DENABLE_pybind11:BOOL=ON \
    -DUSE_SYSTEM_qt5:BOOL=ON \
    -DENABLE_xmsmesher:BOOL=ON \
    $sccache_settings \
    "-D__BUILDBOT_INSTALL_LOCATION:PATH=$SUPERBUILD_PREFIX" \
    "$workdir"
ninja
cp smtk-developer-config.cmake "$SUPERBUILD_PREFIX"

if [ -x "$sccache_mountpoint" ]; then
    "$sccache_mountpoint" --show-stats
fi

rm -rf "$workdir"
