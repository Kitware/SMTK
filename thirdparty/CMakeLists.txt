add_subdirectory(cJSON)
# Only use the local sparsehash when the system version is not requested.
if (NOT SMTK_USE_SYSTEM_SPARSEHASH)
  add_subdirectory(sparsehash)
endif()
