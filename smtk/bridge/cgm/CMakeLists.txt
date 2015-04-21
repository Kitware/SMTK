include_directories(${CGM_INCLUDE_DIRS})

set(cgmSrcs
  Session.cxx
  CAUUID.cxx
  Engines.cxx
  ExportSolid.cxx
  ImportSolid.cxx
  Operator.cxx
  operators/BooleanIntersection.cxx
  operators/BooleanSubtraction.cxx
  operators/BooleanUnion.cxx
  operators/Copy.cxx
  operators/CreateBody.cxx
  operators/CreateBrick.cxx
  operators/CreateCylinder.cxx
  operators/CreateEdge.cxx
  operators/CreateFace.cxx
  operators/CreatePrism.cxx
  operators/CreateSphere.cxx
  operators/CreateVertex.cxx
  operators/Read.cxx
  operators/Reflect.cxx
  operators/Rotate.cxx
  operators/Scale.cxx
  operators/Sweep.cxx
  operators/Translate.cxx
  operators/Write.cxx
  TDUUID.cxx
)

set(cgmHeaders
  Session.h
  CAUUID.h
  Engines.h
  ExportSolid.h
  ImportSolid.h
  Operator.h
  operators/BooleanIntersection.h
  operators/BooleanSubtraction.h
  operators/BooleanUnion.h
  operators/Copy.h
  operators/CreateBody.h
  operators/CreateBrick.h
  operators/CreateCylinder.h
  operators/CreateEdge.h
  operators/CreateFace.h
  operators/CreatePrism.h
  operators/CreateSphere.h
  operators/CreateVertex.h
  operators/Read.h
  operators/Reflect.h
  operators/Rotate.h
  operators/Scale.h
  operators/Sweep.h
  operators/Translate.h
  operators/Write.h
  TDUUID.h
)

install(FILES PointerDefs.h DESTINATION include/smtk/bridge/cgm)

add_definitions(${CGM_DEFINES})

add_library(smtkCGMSession ${cgmSrcs})

#set smtkCGMSession to publicly link to smtkCore and CGM
target_link_libraries(smtkCGMSession
  LINK_PUBLIC
    smtkCore
  LINK_PRIVATE
    ${CGM_LIBRARIES}
  )
smtk_export_header(smtkCGMSession Exports.h)

# Add compile definitions and include directories
set_property(TARGET smtkCGMSession APPEND PROPERTY COMPILE_FLAGS ${CGM_DEFINES})
set_property(TARGET smtkCGMSession APPEND PROPERTY INCLUDE_DIRECTORIES ${CGM_INCLUDE_DIRS})

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(smtkCGMSession PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

#install the library and exports the library when used from a build tree
smtk_install_library(smtkCGMSession DEPENDS smtkCore)

# Normally we would uncomment this line:
#    smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" cgmSessionJSON)
# to generate a header file containing the JSON string defining
# tags for the CGM session. However, because we wish to omit portions
# of the JSON depending on configuration options, a hand-edited header
# is included in the source tree instead. The other option would be to
# build a program at configure-time to conditionally create the JSON and/or header.
# But this is simpler.

# Operators which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to cgmOperatorXML) since the operators themselves include
# the header in their implementations.
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/BooleanIntersection.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/BooleanSubtraction.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/BooleanUnion.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Copy.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateBody.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateBrick.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateCylinder.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateEdge.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateFace.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreatePrism.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateSphere.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateVertex.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Read.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Reflect.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Rotate.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Scale.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Sweep.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Translate.sbt" cgmOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Write.sbt" cgmOperatorXML)

# Install the headers
smtk_public_headers(${cgmHeaders})

# Utility programs using CGM:
add_executable(cgm-convert cgm-convert.cxx)
target_link_libraries(cgm-convert smtkCore smtkCGMSession ${CGM_LIBRARIES})

add_executable(facet-convert facet-convert.cxx)
target_link_libraries(facet-convert smtkCore ${CGM_LIBRARIES})

if (SMTK_BUILD_VTK)
  include("${VTK_USE_FILE}")
  add_executable(vtk-to-cholla vtk-to-cholla.cxx)
  target_link_libraries(vtk-to-cholla
    smtkCore
    vtkCommonDataModel
    vtkIOXML
    ${CGM_LIBRARIES}
  )
endif()


if(SMTK_BUILD_PYTHON_WRAPPINGS AND Shiboken_FOUND)
  #extract the headers from cgm library we built to give them to shiboken

  sbk_wrap_library(smtkCGMSession
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${SMTK_SOURCE_DIR}/smtk
    LOCAL_INCLUDE_DIRECTORIES
      ${SMTK_SOURCE_DIR}/smtk/common
      ${SMTK_SOURCE_DIR}/smtk/attribute
      ${SMTK_SOURCE_DIR}/smtk/model
      ${SMTK_SOURCE_DIR}/smtk/session
      ${SMTK_SOURCE_DIR}/smtk/bridge/cgm
      ${SMTK_SOURCE_DIR}/smtk/simulation
      ${SMTK_SOURCE_DIR}/smtk/io
      ${SMTK_SOURCE_DIR}/smtk/view
      ${SMTK_SOURCE_DIR}/smtk
      ${SMTK_BINARY_DIR}/smtk
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/typesystem.xml
    HEADERS ${cgmHeaders}
    DEPENDS smtkCore
  )
  target_include_directories(smtkCGMSessionPython
    PRIVATE ${CGM_INCLUDE_DIRS})
endif()

if(SMTK_BUILD_SESSION_PLUGIN)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_bridge_cgm LINK_PRIVATE ${CGM_LIBRARIES})
  add_subdirectory(testing)
endif()
