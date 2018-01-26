set(meshSrcs
  Facade.cxx
  RegisterSession.cxx
  Resource.cxx
  Session.cxx
  Topology.cxx
  operators/EulerCharacteristicRatio.cxx
  operators/ImportOperator.cxx
  operators/ExportOperator.cxx
)

set(meshHeaders
  Facade.h
  RegisterSession.h
  Resource.h
  Session.h
  Topology.h
  operators/EulerCharacteristicRatio.h
  operators/ImportOperator.h
  operators/ExportOperator.h
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

# Operators which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to meshOperatorXML) since the operators themselves include
# the header in their implementations.
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EulerCharacteristicRatio.sbt" meshOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ImportOperator.sbt" meshOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ExportOperator.sbt" meshOperatorXML)

# Install the headers
smtk_public_headers(${meshHeaders})

add_subdirectory(environment)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
add_subdirectory(testing)
endif()
