set(srcs
  RedirectOutput.cxx
  vtkLIDARPtsWriter.cxx # For terrain extraction
  mesh/ExportVTKData.cxx
  mesh/ImportVTKData.cxx
  mesh/MeshIOVTK.cxx
  )

set(headers
  RedirectOutput.h
  vtkLIDARPtsWriter.h # For terrain extraction
  mesh/ExportVTKData.h
  mesh/ImportVTKData.h
  mesh/MeshIOVTK.h
  )

add_library(smtkIOVTK ${srcs})

target_link_libraries(smtkIOVTK
  LINK_PUBLIC smtkCore vtkCommonCore vtkCommonDataModel vtkIOCore vtkIOLegacy vtkIOXML vtksys vtkFiltersCore)

smtk_export_header(smtkIOVTK IOVTKExports.h)

target_include_directories(smtkIOVTK
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(smtkIOVTK
  PRIVATE ${MOAB_INCLUDE_DIRS})

#install the headers
smtk_public_headers(${headers})
# ... and make header compilation tests link properly:
smtk_install_library(smtkIOVTK)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
