# Add the Qt GUI code if enabled
if(SMTK_ENABLE_QT_SUPPORT)
  add_subdirectory(qt)
endif()

set(SMTK_VTK_SUPPORT_ENABLED "False")

if (SMTK_ENABLE_VTK_SUPPORT)
  set(SMTK_VTK_SUPPORT_ENABLED "True")
  add_subdirectory(vtk)

  # These are converters to allow importing of vtk data into smtk mesh system.
  # We keep this out of the common vtk model for 2 primary reasons.
  # The primary being is that this library is focused on bringing data into
  # smtk, not rendering, and secondly we don't want to require vtk rendering
  #to be linked for IO routines.
  add_subdirectory(vtk/io)
endif()

set(SMTK_PARAVIEW_SUPPORT_ENABLED "False")

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  set(SMTK_PARAVIEW_SUPPORT_ENABLED "True")
  add_subdirectory(paraview)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()
