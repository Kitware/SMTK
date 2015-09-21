set(vtkToMeshSources
    VTKDataConverter.cxx
    )

set(vtkToMeshHeaders
    VTKDataConverter.h
    )

add_library(vtkToSMTKMesh ${vtkToMeshSources})

target_link_libraries(vtkToSMTKMesh
      LINK_PUBLIC smtkCore vtkCommonDataModel )

smtk_export_header(vtkToSMTKMesh vtkToSMTKMeshExports.h)

target_include_directories(vtkToSMTKMesh
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(vtkToSMTKMesh
  PRIVATE ${MOAB_INCLUDE_DIRS})

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(vtkToSMTKMesh PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

#install the headers
smtk_public_headers(${vtkToMeshHeaders})
# ... and make header compilation tests link properly:
smtk_install_library(vtkToSMTKMesh)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
