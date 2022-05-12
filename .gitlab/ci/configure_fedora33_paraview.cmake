set(SMTK_PLUGIN_CONTRACT_FILE_URLS
  "https://gitlab.kitware.com/cmb/plugins/opencascade-session/-/raw/for/smtk/22.04/cmake/opencascade-smtk-contract.cmake"
  "https://gitlab.kitware.com/cmb/plugins/adh-extensions/-/raw/master/CMake/adh-extensions.cmake"
  "https://gitlab.kitware.com/cmb/plugins/rgg-session/-/raw/master/CMake/rgg-session.cmake"
  "https://gitlab.kitware.com/cmb/plugins/truchas-extensions/-/raw/master/CMake/truchas-extensions.cmake"
  "https://gitlab.kitware.com/cmb/cmb/-/raw/master/cmake/cmb.cmake"
  "https://gitlab.kitware.com/cmb/plugins/ace3p-extensions/-/raw/master/CMake/ace3p-extensions.cmake"
  "https://gitlab.kitware.com/cmb/plugins/xmsmeshoperation/-/raw/master/CMake/xms-mesh-operation.cmake"
  CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora33.cmake")
