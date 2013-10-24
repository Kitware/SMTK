project(cJSON)

add_library(cJSON STATIC cJSON.c)
if (BUILD_SHARED_LIBS OR SMTK_BUILD_PYTHON_WRAPPINGS)
  # Set position-independent flag when other project libs are shared.
  set_target_properties(cJSON PROPERTIES POSITION_INDEPENDENT_CODE True)
endif()
smtk_install_library(cJSON)
