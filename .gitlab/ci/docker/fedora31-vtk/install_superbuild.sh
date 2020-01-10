#!/bin/sh

set -e
set -x

readonly superbuild_ref="$1"
shift

readonly workdir="$HOME/code/cmb"
git clone --recursive https://gitlab.kitware.com/cmb/cmb-superbuild.git "$workdir/src-sb"
git -C "$workdir/src-sb" checkout "$superbuild_ref"
git -C "$workdir/src-sb" submodule update --recursive --init
mkdir -p "$workdir/build-sb"
cd "$workdir/build-sb"

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
    -DENABLE_smtkprojectmanager:BOOL=OFF \
    -DENABLE_smtkresourcemanagerstate:BOOL=OFF \
    -DENABLE_vtkonly:BOOL=ON \
    -DENABLE_python3:BOOL=ON \
    -DSUPPRESS_szip_OUTPUT:BOOL=OFF \
    -DUSE_SYSTEM_qt5:BOOL=ON \
    $sccache_settings \
    "-D__BUILDBOT_INSTALL_LOCATION:PATH=$SUPERBUILD_PREFIX" \
    "$workdir/src-sb"
ninja
cp smtk-developer-config.cmake "$SUPERBUILD_PREFIX"

if [ -x "$sccache_mountpoint" ]; then
    "$sccache_mountpoint" --show-stats
fi

rm -rf "$workdir"
