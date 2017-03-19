################################################################################
# Build widgets
################################################################################
if (PARAVIEW_BUILD_QT_GUI)
  add_subdirectory(widgets)
  add_subdirectory(appcomponents)
  add_subdirectory(operators)
endif()
