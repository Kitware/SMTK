set(meshSrcs
  Facade.cxx
  Registrar.cxx
  Resource.cxx
  Session.cxx
  Topology.cxx
  operators/CreateUniformGrid.cxx
  operators/EulerCharacteristicRatio.cxx
  operators/Import.cxx
  operators/Export.cxx
  operators/Read.cxx
  operators/Write.cxx
)

set(meshHeaders
  Facade.h
  Registrar.h
  Resource.h
  Session.h
  Topology.h
  operators/CreateUniformGrid.h
  operators/EulerCharacteristicRatio.h
  operators/Import.h
  operators/Export.h
  operators/Read.h
  operators/Write.h
)

add_library(smtkMeshSession ${meshSrcs})

target_link_libraries(smtkMeshSession
  LINK_PUBLIC
    smtkCore
  LINK_PRIVATE
    ${__dependencies}
)

smtk_export_header(smtkMeshSession Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkMeshSession)

# Operations which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to meshOperationXML) since the operators themselves include
# the header in their implementations.
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateUniformGrid.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EulerCharacteristicRatio.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Import.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Export.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Read.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Write.sbt" meshOperationXML)

# Install the headers
smtk_public_headers(${meshHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_smtk_plugin(
    smtkMeshSessionPlugin "1.0"
    REGISTRAR smtk::session::mesh::Registrar
    MANAGERS smtk::operation::Manager smtk::resource::Manager
    LIBRARIES smtkCore smtkMeshSession)
  smtk_install_library(smtkMeshSessionPlugin)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
add_subdirectory(testing)
endif()
