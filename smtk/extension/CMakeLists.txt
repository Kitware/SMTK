
#add the Qt GUI code if enabled
if(SMTK_BUILD_QT)
  add_subdirectory(qt)
endif()


if(SMTK_BUILD_VTK)
  # Only include the VTK use-file in subdirectories
  # that require VTK include paths. Otherwise, command
  # lines get too long on some platforms.
  include("${VTK_USE_FILE}")
  list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")
  set(VTK_MAKE_INSTANTIATORS TRUE CACHE INTERNAL "" FORCE)

  # Don't add the VTK or other subdirectories as they use the
  # vtk_module_library() macro and vtk/module.cmake file.
  # The pv_process_modules() macro finds all subdirectories
  # containing a module.cmake file automatically.
  include (vtkSMTKModules)
  vtk_smtk_setup_module_environment("VTKSMTK")
  vtk_smtk_process_modules()

endif()
