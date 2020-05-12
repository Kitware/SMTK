set(SMTK_PLUGIN_CONTRACT_FILE_URLS
  "https://gitlab.kitware.com/cmb/plugins/adh-extensions/-/raw/master/CMake/adh-extensions.cmake"
  "https://gitlab.kitware.com/cmb/plugins/rgg-session/raw/master/CMake/rgg-session.cmake"
  "https://gitlab.kitware.com/cmb/plugins/project-manager/raw/master/CMake/project-manager.cmake"
  "https://gitlab.kitware.com/cmb/plugins/truchas-extensions/raw/master/CMake/truchas-extensions.cmake"
  "https://gitlab.kitware.com/cmb/cmb/raw/master/cmake/cmb.cmake"
  CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora31.cmake")
