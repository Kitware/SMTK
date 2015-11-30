#!/bin/sh

mkdir ~/smtk-build && cd ~/smtk-build
cmake \
  -G Ninja \
  "-DSITE:STRING=travis-ci.org" \
  "-DBUILDNAME:STRING=${TRAVIS_OS_NAME}-${CC}-Job.${TRAVIS_JOB_NUMBER}-SMTK" \
  -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo \
  -DBUILD_SHARED_LIBS:BOOL=ON \
  -DSMTK_ENABLE_QT_SUPPORT:BOOL=ON \
  -DSMTK_ENABLE_DOCUMENTATION:BOOL=ON \
  -DSMTK_ENABLE_TESTING:BOOL=ON \
  -DSMTK_ENABLE_CGM_SESSIONA:BOOL=ON \
  -DCGM_CFG:FILEPATH=/usr/include/cgm.make \
  -DSMTK_ENABLE_PYTHON_WRAPPING:BOOL=ON \
  "-DShiboken_DIR:PATH=${HOME}/smtk-deps/shiboken/install/lib/cmake/Shiboken-1.2.1" \
  -DSMTK_NO_SYSTEM_BOOST:BOOL=OFF \
  "-DCMAKE_INSTALL_PREFIX=${HOME}/smtk-install" \
  ${TRAVIS_BUILD_DIR}
ninja -j2 ExperimentalStart
ninja -j2 ExperimentalConfigure
ninja -j2 ExperimentalBuild
ninja -j2 doc-userguide &>/dev/null
ninja -j2 install
ninja -j2 ExperimentalTest
ninja -j2 ExperimentalSubmit
