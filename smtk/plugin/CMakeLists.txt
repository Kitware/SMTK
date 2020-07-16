set(pluginSrcs
  ClientBase.cxx
  Manager.cxx
  Registry.cxx
)

set(pluginHeaders
  ClientBase.h
  Client.h
  Client.txx
  Manager.h
  Manager.txx
  Registry.h
)

smtk_public_headers(smtkCore ${pluginHeaders})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif ()
