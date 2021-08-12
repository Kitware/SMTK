set(taskSrcs
  Active.cxx
  FillOutAttributes.cxx
  Manager.cxx
  Registrar.cxx
  Task.cxx
  GatherResources.cxx
  json/Helper.cxx
  json/jsonFillOutAttributes.cxx
  json/jsonGatherResources.cxx
  json/jsonManager.cxx
  json/jsonTask.cxx
)

set(taskHeaders
  Active.h
  FillOutAttributes.h
  Instances.h
  Manager.h
  Registrar.h
  Task.h
  GatherResources.h
  json/Helper.h
  json/jsonFillOutAttributes.h
  json/jsonGatherResources.h
  json/jsonManager.h
  json/jsonTask.h
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
