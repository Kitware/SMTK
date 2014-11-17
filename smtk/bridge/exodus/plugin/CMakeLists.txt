if (SMTK_ENABLE_EXODUS_BRIDGE)

  find_package(ParaView REQUIRED)
  include(${PARAVIEW_USE_FILE})

  include (ParaViewPlugins)
  # We need to add the current value of VTK_MODULES_DIR to the module path
  # so that when the plugins are built all the modules can be found. Otherwise,
  # modules that aren't loaded as direct dependencies of CMB modules will
  # not be found.
  list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

  add_paraview_plugin(smtkExodusBridge_Plugin "1.0"
      SERVER_SOURCES
        bridgeInit.cxx
    )

  target_link_libraries(smtkExodusBridge_Plugin
    LINK_PUBLIC
      SMTKCore
      SMTKBridgeExodus
      vtkPVServerManagerApplication
    LINK_PRIVATE
      vtkPVServerManagerApplicationCS
  )
  smtk_install_library(smtkExodusBridge_Plugin DEPENDS SMTKCore SMTKBridgeExodus)
endif()
