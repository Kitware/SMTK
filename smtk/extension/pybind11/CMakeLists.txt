set(build_path "${PROJECT_BINARY_DIR}/smtk/extension")
set(install_path "${SMTK_PYTHON_MODULEDIR}/smtk/extension")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
  "${build_path}/__init__.py" @ONLY
  )

install(
  FILES "${build_path}/__init__.py"
  DESTINATION "${install_path}"
  )
