################################################################################
# Build widgets
################################################################################
if (PARAVIEW_BUILD_QT_GUI)
  add_subdirectory(widgets)
  add_subdirectory(appcomponents)
  add_subdirectory(operators)
endif()
add_subdirectory(server)
add_subdirectory(pluginsupport)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()
