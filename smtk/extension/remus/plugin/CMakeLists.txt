if (SMTK_ENABLE_DISCRETE_SESSION AND SMTK_ENABLE_PARAVIEW_SUPPORT)

  add_paraview_plugin(smtkRemusMeshOperationPlugin "1.0"
    SERVER_SOURCES
      sessionInit.cxx
  )

  target_link_libraries(smtkRemusMeshOperationPlugin
    LINK_PUBLIC
      smtkRemusExt
    LINK_PRIVATE
      vtkPVServerManagerApplicationCS
  )
  smtk_install_library(smtkRemusMeshOperationPlugin DEPENDS smtkRemusExt)
endif()
