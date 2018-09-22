#!/bin/bash

# Side note: need to add conda-forge to the set of archives
# From root directory:
# > conda build conda/meta.yaml -c defaults -c conda-forge

cmake \
  -DCONDA_BUILD=ON \
  -DSMTK_ENABLE_TESTING=OFF \
  -DCMAKE_INSTALL_PREFIX=$PREFIX \
  "${SRC_DIR}"
make -j"${CPU_COUNT}"
make install
