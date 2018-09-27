set(srcs
  ImportAsVTKData.cxx
  RedirectOutput.cxx
  vtkLIDARPtsWriter.cxx # For terrain extraction
  mesh/ExportVTKData.cxx
  mesh/ImportVTKData.cxx
  mesh/MeshIOVTK.cxx
  mesh/PointCloudFromVTKFile.cxx
  mesh/StructuredGridFromVTKFile.cxx
  )

set(headers
  ImportAsVTKData.h
  RedirectOutput.h
  vtkLIDARPtsWriter.h # For terrain extraction
  mesh/ExportVTKData.h
  mesh/ImportVTKData.h
  mesh/MeshIOVTK.h
  mesh/PointCloudFromVTKFile.h
  mesh/StructuredGridFromVTKFile.h
  )

add_library(smtkIOVTK ${srcs})

target_link_libraries(smtkIOVTK
  LINK_PUBLIC smtkCore vtkSMTKFilterExt vtkSMTKReaderExt vtkCommonCore vtkCommonDataModel vtkFiltersGeneral vtkIOCore vtkIOGDAL vtkInteractionStyle vtkIOGeometry vtkIOImage vtkIOLegacy vtkIOPLY vtkIOXML vtkImagingCore vtkRenderingFreeType vtkRenderingOpenGL2 vtksys vtkFiltersCore)

smtk_export_header(smtkIOVTK IOVTKExports.h)

target_include_directories(smtkIOVTK
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(smtkIOVTK
  PRIVATE ${MOAB_INCLUDE_DIRS})

#install the headers
smtk_public_headers(smtkIOVTK ${headers})
# ... and make header compilation tests link properly:
smtk_install_library(smtkIOVTK)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
