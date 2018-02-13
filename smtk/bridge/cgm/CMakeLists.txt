include_directories(${CGM_INCLUDE_DIRS})

set(cgmSrcs
  Session.cxx
  CAUUID.cxx
  Engines.cxx
  ExportSolid.cxx
  ImportSolid.cxx
  Operation.cxx
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
  operators/RemoveModel.cxx
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
  Operation.h
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
  operators/RemoveModel.h
  operators/Rotate.h
  operators/Scale.h
  operators/Sweep.h
  operators/Translate.h
  operators/Write.h
  TDUUID.h
)

install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/bridge/cgm)

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

#install the library and exports the library when used from a build tree
smtk_install_library(smtkCGMSession)

# Normally we would uncomment this line:
#    smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" cgmSessionJSON)
# to generate a header file containing the JSON string defining
# tags for the CGM session. However, because we wish to omit portions
# of the JSON depending on configuration options, a hand-edited header
# is included in the source tree instead. The other option would be to
# build a program at configure-time to conditionally create the JSON and/or header.
# But this is simpler.

# Operations which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to cgmOperationXML) since the operators themselves include
# the header in their implementations.
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/BooleanIntersection.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/BooleanSubtraction.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/BooleanUnion.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Copy.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateBody.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateBrick.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateCylinder.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateEdge.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateFace.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreatePrism.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateSphere.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateVertex.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Read.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Reflect.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/RemoveModel.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Rotate.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Scale.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Sweep.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Translate.sbt" cgmOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Write.sbt" cgmOperationXML)

# Install the headers
smtk_public_headers(${cgmHeaders})

# Utility programs using CGM:
add_executable(cgm-convert cgm-convert.cxx)
target_link_libraries(cgm-convert smtkCore smtkCGMSession ${CGM_LIBRARIES})

add_executable(facet-convert facet-convert.cxx)
target_link_libraries(facet-convert smtkCore ${CGM_LIBRARIES})

if (SMTK_ENABLE_VTK_SUPPORT)
  include("${VTK_USE_FILE}")
  add_executable(vtk-to-cholla vtk-to-cholla.cxx)
  target_link_libraries(vtk-to-cholla
    smtkCore
    vtkCommonDataModel
    vtkIOXML
    ${CGM_LIBRARIES}
  )
endif()

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_bridge_cgm LINK_PRIVATE ${CGM_LIBRARIES})
  add_subdirectory(testing)
endif()
