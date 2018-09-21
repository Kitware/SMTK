#!/bin/bash

# Side note: need to add conda-forge to the set of archives
# From root directory:
# > conda build conda/meta.yaml -c defaults -c conda-forge  --python=2.7

build_dir=`pwd`

# Patch moab on linux
if [ "$(uname -s)" = "Linux" ]; then
  echo Patching MOAB
  cd ${SRC_DIR}/moab
  git apply ${SRC_DIR}/smtk/conda/moab-src-io-mhdf-CMakeLists.txt.linux.patch
fi

# Build MOAB
mkdir -p ${build_dir}/build/moab
cd ${build_dir}/build/moab
cmake \
  -C "${SRC_DIR}/smtk/conda/moab.cmake" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$PREFIX \
  -DCMAKE_TOOLCHAIN_FILE=$SRC_DIR/smtk/conda/CondaToolchain.cmake \
  "${SRC_DIR}/moab"
cmake --build . -j "${CPU_COUNT}" --target MOAB
cmake --build . --target install

# Build SMTK
mkdir -p ${build_dir}/build/smtk
cd ${build_dir}/build/smtk
cmake \
  -DCONDA_BUILD=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DSMTK_ENABLE_TESTING=OFF \
  -DCMAKE_INSTALL_PREFIX=$PREFIX \
  "${SRC_DIR}/smtk"
cmake --build . -j "${CPU_COUNT}"
cmake --build . --target install
