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
  if (${Boost_MAJOR_VERSION} LESS 2 AND ${Boost_MINOR_VERSION} LESS 52)
    message(FATAL_ERROR
      "Polygon session requires Boost 1.52 or newer, found ${Boost_VERSION}")
  endif()
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
if (SMTK_ENABLE_REMOTE_SESSION AND REMUS_FOUND)
  add_subdirectory(remote)
endif()
