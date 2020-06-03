# Add the Qt GUI code if enabled
if(SMTK_ENABLE_QT_SUPPORT)
  add_subdirectory(qt)
endif()

# Add operators that use Delaunay 2d meshing
add_subdirectory(delaunay)

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

# XXX: Disabled because it needs ported from cJSON to nlohmann_json
# Add the cumulus UI if Qt support and cumulus enabled
#if(SMTK_ENABLE_QT_SUPPORT AND SMTK_ENABLE_CUMULUS_SUPPORT)
#  add_subdirectory(cumulus)
#endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)

  option(SMTK_ENABLE_MATPLOTLIB "Enable matplotlib" OFF)
  set(SMTK_MATPLOTLIB_SUPPORT_ENABLED "False")
  if(SMTK_ENABLE_MATPLOTLIB)
    set(SMTK_MATPLOTLIB_SUPPORT_ENABLED "True")
    add_subdirectory(matplotlib)
  endif()

  add_subdirectory(pybind11)
endif()
