set(meshSrcs
  Session.cxx
  Operator.cxx
  ReadOperator.cxx
  Topology.cxx
  WriteOperator.cxx
)

set(meshHeaders
  Session.h
  Operator.h
  PointerDefs.h
  ReadOperator.h
  Topology.h
  WriteOperator.h
)

add_library(smtkMeshSession ${meshSrcs})

target_link_libraries(smtkMeshSession
  LINK_PUBLIC
    smtkCore
  LINK_PRIVATE
    ${__dependencies}
)

smtk_export_header(smtkMeshSession Exports.h)

# Add compile definitions and include directories
#set_property(TARGET smtkMeshSession APPEND PROPERTY COMPILE_FLAGS ${MESH_DEFINES})
#set_property(TARGET smtkMeshSession APPEND PROPERTY INCLUDE_DIRECTORIES ${MESH_INCLUDE_DIRS})

#install the library and exports the library when used from a build tree
smtk_install_library(smtkMeshSession)

# Operators which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to meshOperatorXML) since the operators themselves include
# the header in their implementations.
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/ReadOperator.sbt" meshOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/WriteOperator.sbt" meshOperatorXML)
smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" meshSessionJSON)

# Install the headers
smtk_public_headers(${meshHeaders})

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)
  #extract the headers from mesh library we built to give them to shiboken

  sbk_wrap_library(smtkMeshSession
    PACKAGE smtk
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${SMTK_SOURCE_DIR}/smtk
    LOCAL_INCLUDE_DIRECTORIES
      ${SMTK_SOURCE_DIR}/smtk/common
      ${SMTK_SOURCE_DIR}/smtk/attribute
      ${SMTK_SOURCE_DIR}/smtk/model
      ${SMTK_SOURCE_DIR}/smtk/session
      ${SMTK_SOURCE_DIR}/smtk/bridge/mesh
      ${SMTK_SOURCE_DIR}/smtk/simulation
      ${SMTK_SOURCE_DIR}/smtk/io
      ${SMTK_SOURCE_DIR}/smtk/view
      ${SMTK_SOURCE_DIR}/smtk
      ${SMTK_BINARY_DIR}/smtk
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/typesystem.xml
    HEADERS ${meshHeaders}
    DEPENDS smtkCore
  )
target_include_directories(smtkMeshSessionPython
    PRIVATE ${MESH_INCLUDE_DIRS})
endif()

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_TESTING)
add_subdirectory(testing)
endif()
