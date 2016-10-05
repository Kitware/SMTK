################################################################################
# Build cgm session
################################################################################
if (SMTK_ENABLE_CGM_SESSION AND CGM_FOUND)
  add_subdirectory(cgm)
endif()

################################################################################
# Build Discrete session
################################################################################
if(SMTK_ENABLE_DISCRETE_SESSION)
  add_subdirectory(discrete)
endif()

################################################################################
# Build Boost.Polygon session
################################################################################
if (SMTK_ENABLE_POLYGON_SESSION)
  add_subdirectory(polygon)
endif()

################################################################################
# Build Exodus-II session
################################################################################
if (SMTK_ENABLE_EXODUS_SESSION)
  add_subdirectory(exodus)
endif()

################################################################################
# Build remus remote session
################################################################################
# This must come *after* all other targets that implement sessions.
if (SMTK_ENABLE_REMOTE_SESSION AND Remus_FOUND)
  add_subdirectory(remote)
endif()

################################################################################
# Build smtk::mesh session
################################################################################
if (SMTK_ENABLE_MESH_SESSION)
  add_subdirectory(mesh)
endif()
