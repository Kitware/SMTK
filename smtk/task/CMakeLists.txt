set(taskSrcs
  Active.cxx
  Manager.cxx
  Registrar.cxx
  Task.cxx
  TaskNeedsResources.cxx
  json/Helper.cxx
  json/jsonManager.cxx
  json/jsonTask.cxx
  json/jsonTaskNeedsResources.cxx
)

set(taskHeaders
  Active.h
  Instances.h
  Manager.h
  Registrar.h
  Task.h
  TaskNeedsResources.h
  json/Helper.h
  json/jsonManager.h
  json/jsonTask.h
  json/jsonTaskNeedsResources.h
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
