
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
# Build Exodus-II session
################################################################################
if (SMTK_ENABLE_EXODUS_SESSION)
  add_subdirectory(exodus)
endif()

################################################################################
# Build remus remote session
################################################################################
# This must come *after* all targets that implement sessions.
if (SMTK_ENABLE_REMUS)
  add_subdirectory(remote)
endif()
