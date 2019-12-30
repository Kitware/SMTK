################################################################################
# Build vxl
################################################################################
add_subdirectory(operators)
add_subdirectory(widgets)

set(module_files
  "${CMAKE_CURRENT_SOURCE_DIR}/vtk/vtk.module")
vtk_module_scan(
  MODULE_FILES ${module_files}
  PROVIDES_MODULES vtk_modules
  HIDE_MODULES_FROM_CACHE ON
  WANT_BY_DEFAULT ON)
vtk_module_build(
  MODULES ${vtk_modules}
  PACKAGE SMTKVXLExt
  INSTALL_EXPORT SMTKVXLExt
  CMAKE_DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
  HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${PROJECT_VERSION}"
  TEST_DIRECTORY_NAME "NONE")
