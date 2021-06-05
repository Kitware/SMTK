set(taskSrcs
  Manager.cxx
  Registrar.cxx
  Task.cxx
  TaskNeedsResources.cxx
)

set(taskHeaders
  Factory.h
  Manager.h
  Registrar.h
  Task.h
  TaskNeedsResources.h
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(smtkCore ${taskHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
