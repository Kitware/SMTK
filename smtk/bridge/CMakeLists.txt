################################################################################
# Initialize flags for configuring Pybind11 bridge module
################################################################################
set(SMTK_CGM_SESSION_ENABLED "False")
set(SMTK_DISCRETE_SESSION_ENABLED "False")
set(SMTK_POLYGON_SESSION_ENABLED "False")
set(SMTK_MESH_SESSION_ENABLED "False")
set(SMTK_MULTISCALE_SESSION_ENABLED "False")

################################################################################
# Build cgm session
################################################################################
# Temporarily disable CGM session until it is updated to the new operator system
#if (SMTK_ENABLE_CGM_SESSION AND CGM_FOUND)
#  set(SMTK_CGM_SESSION_ENABLED "True")
#  add_subdirectory(cgm)
#endif()

################################################################################
# Build Discrete session
################################################################################
if(SMTK_ENABLE_DISCRETE_SESSION)
  set(SMTK_DISCRETE_SESSION_ENABLED "True")
  add_subdirectory(discrete)
endif()

################################################################################
# Build Boost.Polygon session
################################################################################
if (SMTK_ENABLE_POLYGON_SESSION)
  set(SMTK_POLYGON_SESSION_ENABLED "True")
  add_subdirectory(polygon)
endif()

################################################################################
# Build smtk::mesh session
################################################################################
if (SMTK_ENABLE_MESH_SESSION)
  set(SMTK_MESH_SESSION_ENABLED "True")
  add_subdirectory(mesh)
endif()

################################################################################
# Build Multiscale session
################################################################################
if (SMTK_ENABLE_MULTISCALE_SESSION)
  set(SMTK_MULTISCALE_SESSION_ENABLED "True")
  add_subdirectory(multiscale)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()
