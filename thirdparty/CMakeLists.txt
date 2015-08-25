add_subdirectory(cJSON)
if (NOT SMTK_USE_SYSTEM_MOAB)
  add_subdirectory(moab)
endif()
# Only use the local sparsehash when the system version is not requested.
if (NOT SMTK_USE_SYSTEM_SPARSEHASH)
  add_subdirectory(sparsehash)
endif()
