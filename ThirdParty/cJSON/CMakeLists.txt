project(cJSON)

add_library(cJSON STATIC cJSON.c)
smtk_install_library(cJSON)

if (BUILD_SHARED_LIBS)
  # Set position-independent flag when other project libs are shared.
  set_target_properties(cJSON PROPERTIES POSITION_INDEPENDENT_CODE True)
endif()
