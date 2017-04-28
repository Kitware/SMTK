if(SMTK_ENABLE_POLYGON_SESSION)
  set(opencvHeaders
    operators/SurfaceExtractContours.h
    )

  set(opencvSrcs
    operators/SurfaceExtractContours.cxx
    )

  smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SurfaceExtractContours.sbt" opencvOperatorXML)

  set(import_vtk_depends)

  # The import operator is only available if there is VTK
  if(SMTK_ENABLE_VTK_SUPPORT)
    include("${VTK_USE_FILE}")
    list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

    # Don't add the VTK or other subdirectories as they use the
    # vtk_module_library() macro and vtk/module.cmake file.
    # The vtk_smtk_process_modules() macro finds all subdirectories
    # containing a module.cmake file automatically.
    include (smtkVTKModules)
    vtk_smtk_setup_module_environment("SMTK")

    list(APPEND import_vtk_depends
      vtkCommonCore
      vtkCommonDataModel
      vtkFiltersGeometry
      vtkInteractionStyle
      vtkIOParallel
      vtkRenderingCore
      vtkRenderingFreeType
      vtkRenderingOpenGL2
      vtkSMTKReaderExt
      vtksys
      )
  endif()

  add_library(smtkOpenCVExt ${opencvSrcs})
  target_link_libraries(smtkOpenCVExt
    LINK_PUBLIC
    smtkCore
    smtkPolygonSession
    LINK_PRIVATE
    ${import_vtk_depends}
    ${Boost_LIBRARIES}
    )

  #vtk targets don't specify an include directory through usage-requirements, so
  #we manually add them here
  set(includes_from_vtk)
  foreach(module ${import_vtk_depends})
    list(APPEND includes_from_vtk "${${module}_INCLUDE_DIRS}")
  endforeach(module)

  target_include_directories(smtkOpenCVExt
    PRIVATE
    ${includes_from_vtk}
    )

  smtk_export_header(smtkOpenCVExt Exports.h)

  # Install the library and exports the library when used from a build tree
  smtk_install_library(smtkOpenCVExt)
  # Install the headers
  smtk_public_headers(${opencvHeaders})

  if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)

    # Extract the headers from polygon library we built to give them to shiboken
    sbk_wrap_library(smtkOpenCVExt
      PACKAGE smtk
      GENERATOR_ARGS --avoid-protected-hack
      WORKING_DIRECTORY ${SMTK_SOURCE_DIR}/smtk
      LOCAL_INCLUDE_DIRECTORIES
      ${SMTK_SOURCE_DIR}/smtk/common
      ${SMTK_SOURCE_DIR}/smtk/attribute
      ${SMTK_SOURCE_DIR}/smtk/model
      ${SMTK_SOURCE_DIR}/smtk/session
      ${SMTK_SOURCE_DIR}/smtk/bridge/polygon
      ${SMTK_SOURCE_DIR}/smtk/simulation
      ${SMTK_SOURCE_DIR}/smtk/io
      ${SMTK_SOURCE_DIR}/smtk/view
      ${SMTK_SOURCE_DIR}/smtk
      ${SMTK_BINARY_DIR}/smtk
      ${CMAKE_CURRENT_BINARY_DIR}
      TYPESYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/typesystem.xml
      HEADERS ${polygonHeaders}
      DEPENDS smtkCore
      )
  endif()

  if(SMTK_ENABLE_PARAVIEW_SUPPORT)
    add_subdirectory(qt)
    add_subdirectory(plugin)
  endif()
endif()
