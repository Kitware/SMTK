################################################################################
# Initialize flags for configuring Pybind11 session module
################################################################################
set(SMTK_POLYGON_SESSION_ENABLED "False")
set(SMTK_VTK_SESSION_ENABLED "False")
set(SMTK_OSCILLATOR_SESSION_ENABLED "False")

################################################################################
# Build Boost.Polygon session
################################################################################
if (SMTK_ENABLE_POLYGON_SESSION)
  set(SMTK_POLYGON_SESSION_ENABLED "True")
  add_subdirectory(polygon)
endif()

################################################################################
# Build VTK session
################################################################################
if (SMTK_ENABLE_VTK_SESSION)
  set(SMTK_VTK_SESSION_ENABLED "True")
  add_subdirectory(vtk)
endif()

################################################################################
# Build Oscillator session
################################################################################
if (SMTK_ENABLE_OSCILLATOR_SESSION)
  set(SMTK_OSCILLATOR_SESSION_ENABLED "True")
  add_subdirectory(oscillator)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()
