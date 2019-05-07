if(SMTK_ENABLE_POLYGON_SESSION)
  set(opencvHeaders
    operators/SurfaceExtractContours.h
    )

  set(opencvSrcs
    operators/SurfaceExtractContours.cxx
    )

  smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SurfaceExtractContours.sbt" opencvOperationXML)

  set(import_vtk_depends)

  # The import operator is only available if there is VTK
  if(SMTK_ENABLE_VTK_SUPPORT)
    list(APPEND import_vtk_depends
      vtkSMTKReaderExt
      VTK::CommonCore
      VTK::CommonDataModel
      VTK::FiltersGeometry
      VTK::IOParallel
      VTK::InteractionStyle
      VTK::RenderingCore
      VTK::RenderingFreeType
      VTK::RenderingOpenGL2
      VTK::vtksys
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
  vtk_module_autolink(
    TARGETS smtkOpenCVExt
    MODULES ${import_vtk_depends})

  smtk_export_header(smtkOpenCVExt Exports.h)

  # Install the library and exports the library when used from a build tree
  smtk_install_library(smtkOpenCVExt)
  # Install the headers
  smtk_public_headers(smtkOpenCVExt ${opencvHeaders})

  if(SMTK_ENABLE_PARAVIEW_SUPPORT)
    add_subdirectory(qt)
    add_subdirectory(plugin)
  endif()
endif()
