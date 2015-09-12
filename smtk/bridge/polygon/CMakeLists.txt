set(polygonSrcs
  Session.cxx
  internal/Model.cxx
  operators/CreateModel.cxx
)

set(polygonHeaders
  Session.h
  internal/Model.h
  operators/CreateModel.h
)

smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" polygonSessionJSON)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateModel.sbt" polygonOperatorXML)

add_library(smtkPolygonSession ${polygonSrcs})
target_link_libraries(smtkPolygonSession
  LINK_PUBLIC
    smtkCore
  )
smtk_export_header(smtkPolygonSession Exports.h)

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(smtkPolygonSession PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# Install the library and exports the library when used from a build tree
smtk_install_library(smtkPolygonSession)
# Install the headers
smtk_public_headers(${polygonHeaders})


install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/bridge/polygon)
target_include_directories(smtkPolygonSession
  PUBLIC
    $<BUILD_INTERFACE:${SMTK_SOURCE_DIR}/smtk/bridge/discrete/extension/meshing>
)

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)

  # Extract the headers from polygon library we built to give them to shiboken
  sbk_wrap_library(smtkPolygonSession
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
  add_subdirectory(plugin)
endif()
