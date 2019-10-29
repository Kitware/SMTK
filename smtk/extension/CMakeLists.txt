# Add the Qt GUI code if enabled
if(SMTK_ENABLE_QT_SUPPORT)
  add_subdirectory(qt)
endif()

# Add mesh operator if Remus is enabled.
#this needs to be above vtk_smtk_setup_module_environment as that
#pollutes the link libraries, and causes the remus plugin to depend
#on libraries it doesn't require
#if (SMTK_ENABLE_REMUS_SUPPORT)
#  add_subdirectory(remus)
#endif()

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

if(SMTK_ENABLE_OPENCV)
  add_subdirectory(opencv)
endif()

set(SMTK_PARAVIEW_SUPPORT_ENABLED "False")

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  set(SMTK_PARAVIEW_SUPPORT_ENABLED "True")
  add_subdirectory(paraview)
endif()

# Add the cumulus UI if Qt support and cumulus enabled
if(SMTK_ENABLE_QT_SUPPORT AND SMTK_ENABLE_CUMULUS_SUPPORT)
  add_subdirectory(cumulus)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)

  option(SMTK_ENABLE_MATPLOTLIB "Enable matplotlib" OFF)
  set(SMTK_MATPLOTLIB_SUPPORT_ENABLED "False")
  if(SMTK_ENABLE_MATPLOTLIB)
    set(SMTK_MATPLOTLIB_SUPPORT_ENABLED "True")
    add_subdirectory(matplotlib)
  endif()

  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_VXL_SUPPORT AND SMTK_ENABLE_QT_SUPPORT)
  add_subdirectory(vxl)
endif()
