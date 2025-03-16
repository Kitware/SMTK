set(SMTK_PLUGIN_CONTRACT_FILE_URLS
  # Disabled until importing into smtk::mesh can be replaced with another resource type.
  # "https://gitlab.kitware.com/cmb/plugins/openfoam-windtunnel/-/raw/master/cmake/openfoam-windtunnel.cmake"
  "https://gitlab.kitware.com/cmb/plugins/opencascade-session/-/raw/master/cmake/opencascade-smtk-contract.cmake"
  # https://gitlab.kitware.com/cmb/smtk/-/issues/543
  # "https://gitlab.kitware.com/cmb/plugins/truchas-extensions/-/raw/master/CMake/truchas-extensions.cmake"
  "https://gitlab.kitware.com/cmb/cmb/-/raw/master/cmake/cmb.cmake"
  # https://gitlab.kitware.com/cmb/smtk/-/issues/543
  # "https://gitlab.kitware.com/cmb/plugins/ace3p-extensions/-/raw/master/CMake/ace3p-extensions.cmake"
  CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora41.cmake")
