set(build_path "${CMAKE_BINARY_DIR}/${SMTK_PYTHON_MODULEDIR}/smtk/extension/vtk")
set(install_path "${SMTK_PYTHON_MODULEDIR}/smtk/extension/vtk")
set(headers
  PybindVTKTypeCaster.h
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
  "${build_path}/__init__.py" @ONLY
)

install(
  FILES "${build_path}/__init__.py"
  DESTINATION "${install_path}"
)

# Install pybind support header for VTK objects.
# NB: The smtkVTKExt passed to smtk_public_headers does not exist...
#     but smtk_public_headers ignores the first argument.
smtk_public_headers(smtkVTKExt ${headers})

add_subdirectory(modules)
