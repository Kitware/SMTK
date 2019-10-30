#!/bin/sh

set -e

readonly workdir="$HOME/code/cmb"
git clone --recursive https://gitlab.kitware.com/cmb/cmb-superbuild.git "$workdir/src-sb"
mkdir -p "$workdir/build-sb"
cd "$workdir/build-sb"

cmake -GNinja \
    -DDEVELOPER_MODE_smtk:BOOL=ON \
    -DENABLE_cmb:BOOL=OFF \
    -DENABLE_cmbusersguide:BOOL=OFF \
    -DENABLE_smtkprojectmanager:BOOL=OFF \
    -DENABLE_smtkresourcemanagerstate:BOOL=OFF \
    -DSUPPRESS_szip_OUTPUT:BOOL=OFF \
    -DUSE_SYSTEM_qt5:BOOL=ON \
    "-D__BUILDBOT_INSTALL_LOCATION:PATH=$HOME/misc/root/smtk-deps" \
    "$workdir/src-sb"
ninja
cp smtk-developer-config.cmake "$HOME/misc/root/smtk-deps"

rm -rf "$workdir"
