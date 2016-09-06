set(vtkToMeshSources
    VTKDataConverter.cxx
    )

set(vtkToMeshHeaders
    VTKDataConverter.h
    )

add_library(vtkToSMTKMesh ${vtkToMeshSources})

target_link_libraries(vtkToSMTKMesh
      LINK_PUBLIC smtkCore vtkCommonDataModel vtkIOXML vtksys )

smtk_export_header(vtkToSMTKMesh vtkToSMTKMeshExports.h)

target_include_directories(vtkToSMTKMesh
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(vtkToSMTKMesh
  PRIVATE ${MOAB_INCLUDE_DIRS})

#install the headers
smtk_public_headers(${vtkToMeshHeaders})
# ... and make header compilation tests link properly:
smtk_install_library(vtkToSMTKMesh)

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)
  #extract the headers from cgm library we built to give them to shiboken

  sbk_wrap_library(vtkToSMTKMesh
    PACKAGE smtk
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${SMTK_SOURCE_DIR}/smtk
    LOCAL_INCLUDE_DIRECTORIES
      ${SMTK_SOURCE_DIR}/smtk/common
      ${SMTK_SOURCE_DIR}/smtk/mesh
      ${SMTK_SOURCE_DIR}/smtk/extension/vtkToMesh
      ${SMTK_SOURCE_DIR}/smtk
      ${SMTK_BINARY_DIR}/smtk
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/typesystem.xml
    HEADERS ${vtkToMeshHeaders}
    DEPENDS smtkCore
  )
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
