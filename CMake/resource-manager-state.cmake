# This file is provided with SMTK as an example of how to configure a plugin
# contract file. In general, SMTK should not contain these files. Instead, the
# dashboard computer that runs SMTK tests should contain a set of plugin
# contract test files; these files should be passed to SMTK during its
# configuration.

cmake_minimum_required(VERSION 2.8)
project(resource-manager-state)

include(ExternalProject)

set(response_file)
if (WIN32)
  # Force response file usage. The command line gets way too long on Windows
  # without this. Once VTK_USE_FILE and PARAVIEW_USE_FILE are gone, this can be
  # removed again.
  set(response_file -DCMAKE_NINJA_FORCE_RESPONSE_FILE:BOOL=ON)
endif ()

ExternalProject_Add(read-and-write-resource-manager-state
  GIT_REPOSITORY "https://gitlab.kitware.com/cmb/plugins/read-and-write-resource-manager-state.git"
  GIT_TAG "origin/master"
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DENABLE_TESTING=ON
    -Dsmtk_DIR=${smtk_DIR}
    ${response_file}
  INSTALL_COMMAND ""
  TEST_BEFORE_INSTALL True
)
