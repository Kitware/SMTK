
#add the Qt GUI code if enabled
if(SMTK_BUILD_QT)
  add_subdirectory(qt)
endif()

if(SMTK_BUILD_VTK)
  if(SMTK_BUILD_BRIDGE_PLUGIN)
    #Only include the ParaView use-file in subdirectories
    # that require ParaView include paths.

    find_package(ParaView REQUIRED)
    include("${PARAVIEW_USE_FILE}")
    pv_setup_module_environment("VTKSMTK")
    pv_process_modules()
  else()
    # Only include the VTK use-file in subdirectories
    # that require VTK include paths. Otherwise, command
    # lines get too long on some platforms.

    # We need the COMPONENTS argument so that this will work
    # with both standalone VTK or Paraview-VTK
    set(__required_vtk_modules
        vtkCommonCore
        vtkCommonDataModel
        vtkCommonExecutionModel
        vtkRenderingCore
        vtkInteractionStyle
        vtkViewsCore
        vtkViewsInfovis
        vtkIOXML
        vtkIOLegacy
        vtkInteractionStyle
        vtkRenderingOpenGL
        vtkRenderingVolumeOpenGL
        vtkRenderingFreeTypeOpenGL
    )

    if (SMTK_ENABLE_TESTING)
      list(APPEND __required_vtk_modules vtkTestingRendering)
    endif()
    message("__required_vtk_modules = ${__required_vtk_modules}")
    find_package( VTK REQUIRED COMPONENTS ${__required_vtk_modules} )

    include("${VTK_USE_FILE}")
    list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")
    # message("VTK_MODULES_DIR=${VTK_MODULES_DIR}")
    set(VTK_MAKE_INSTANTIATORS TRUE CACHE INTERNAL "" FORCE)

    # Don't add the VTK or other subdirectories as they use the
    # vtk_module_library() macro and vtk/module.cmake file.
    # The pv_process_modules() macro finds all subdirectories
    # containing a module.cmake file automatically.
    include (vtkSMTKModules)
    vtk_smtk_setup_module_environment("VTKSMTK")
    vtk_smtk_process_modules()
  endif()
endif()
