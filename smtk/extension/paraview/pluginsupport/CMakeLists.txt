include(${PARAVIEW_USE_FILE})
include (ParaViewPlugins)

# We need to add the current value of VTK_MODULES_DIR to the module path
# so that when the plugins are built all the modules can be found. Otherwise,
# modules that aren't loaded as direct dependencies of CMB modules will
# not be found.
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

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

add_paraview_plugin(smtkPluginSupportPlugin "1.0"
  SERVER_SOURCES sessionInit.cxx
  )

smtk_export_header(smtkPluginSupport Exports.h)

target_link_libraries(smtkPluginSupportPlugin
  LINK_PUBLIC
    smtkCore
    smtkPluginSupport
    vtkPVServerManagerApplication
  LINK_PRIVATE
    vtkPVServerManagerApplicationCS
)
smtk_install_library(smtkPluginSupportPlugin)

smtk_public_headers(${hdrs})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

add_subdirectory(testing)
