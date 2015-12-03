project(cJSON)

add_library(cJSON STATIC cJSON.c)
target_include_directories(cJSON
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/smtk/${SMTK_VERSION}>)

if (BUILD_SHARED_LIBS OR SMTK_ENABLE_PYTHON_WRAPPING)
  # Set position-independent flag when other project libs are shared.
  set_target_properties(cJSON PROPERTIES POSITION_INDEPENDENT_CODE True)
endif()

smtk_install_library(cJSON)
install(FILES cJSON.h DESTINATION include/smtk/${SMTK_VERSION})
