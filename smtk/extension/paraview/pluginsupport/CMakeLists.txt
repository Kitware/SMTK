set(srcs
  PluginClientBase.cxx
  PluginManager.cxx
)

set(hdrs
  PluginClientBase.h
  PluginClient.h
  PluginClient.txx
  PluginManager.h
  PluginManager.txx
)

add_library(smtkPluginSupport
  ${srcs}
)

smtk_install_library(smtkPluginSupport)

smtk_export_header(smtkPluginSupport Exports.h)

smtk_public_headers(smtkPluginSupport ${hdrs})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

add_subdirectory(testing)
