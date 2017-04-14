set(source
  TessellateFace.cxx
  TriangulateFace.cxx)
set(headers
  TessellateFace.h
  TriangulateFace.h)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/TriangulateFace.sbt" delaunayOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/TessellateFace.sbt" delaunayOperatorXML)

add_library(smtkDelaunayExt ${source})
target_link_libraries(smtkDelaunayExt
  PRIVATE
    smtkCore
    DelaunayShape
    DelaunayMesh
    DelaunayMisc
    DelaunayValidation
    DelaunayDiscretization
)

smtk_export_header(smtkDelaunayExt Exports.h)
smtk_public_headers(${headers})
smtk_install_library(smtkDelaunayExt)

add_subdirectory(plugin)

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)

  sbk_wrap_library(smtkDelaunayExt
    PACKAGE smtk
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${SMTK_SOURCE_DIR}/smtk
    LOCAL_INCLUDE_DIRECTORIES
      ${SMTK_SOURCE_DIR}/smtk/attribute
      ${SMTK_SOURCE_DIR}/smtk/common
      ${SMTK_SOURCE_DIR}/smtk/mesh
      ${SMTK_SOURCE_DIR}/smtk/model
      ${SMTK_SOURCE_DIR}/smtk/extension/delaunay
      ${SMTK_SOURCE_DIR}/smtk
      ${SMTK_BINARY_DIR}/smtk
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/typesystem.xml
    HEADERS ${headers}
    DEPENDS smtkCore
  )
endif()

if (SMTK_USE_PYBIND11)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
