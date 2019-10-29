if (SMTK_ENABLE_PARAVIEW_SUPPORT)

  add_paraview_plugin(smtkMatplotlibPlugin "1.0"
    SERVER_SOURCES
      sessionInit.cxx
  )

  target_link_libraries(smtkMatplotlibPlugin
    LINK_PUBLIC
      smtkCore
    LINK_PRIVATE
      vtkPVServerManagerApplicationCS
  )
  smtk_install_library(smtkMatplotlibPlugin DEPENDS smtkCore)
endif()
