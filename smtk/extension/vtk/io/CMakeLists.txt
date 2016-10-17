set(srcs
  MeshIOVTK.cxx
  VTKDataConverter.cxx
  )

set(headers
  MeshIOVTK.h
  VTKDataConverter.h
  )

add_library(IOVTK ${srcs})

target_link_libraries(IOVTK
      LINK_PUBLIC smtkCore vtkCommonDataModel vtkIOXML vtksys )

smtk_export_header(IOVTK IOVTKExports.h)

target_include_directories(IOVTK
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(IOVTK
  PRIVATE ${MOAB_INCLUDE_DIRS})

#install the headers
smtk_public_headers(${headers})
# ... and make header compilation tests link properly:
smtk_install_library(IOVTK)

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)
  #extract the headers from cgm library we built to give them to shiboken

  sbk_wrap_library(IOVTK
    PACKAGE smtk
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${SMTK_SOURCE_DIR}/smtk
    LOCAL_INCLUDE_DIRECTORIES
      ${SMTK_SOURCE_DIR}/smtk/common
      ${SMTK_SOURCE_DIR}/smtk/mesh
      ${SMTK_SOURCE_DIR}/smtk/extension/vtk/io
      ${SMTK_SOURCE_DIR}/smtk
      ${SMTK_BINARY_DIR}/smtk
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/typesystem.xml
    HEADERS ${headers}
    DEPENDS smtkCore
  )
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
