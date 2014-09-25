
################################################################################
# Build cgm bridge
################################################################################
if (SMTK_BUILD_CGM AND CGM_FOUND)
  add_subdirectory(cgm)
endif()

################################################################################
# Build remus remote bridge
################################################################################
# This must come *after* all targets that implement bridges.
if (SMTK_ENABLE_REMUS)
  add_subdirectory(remote)
endif()
