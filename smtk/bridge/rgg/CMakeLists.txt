set (rggSrcs
  Operator.cxx
  Session.cxx
  operators/CreateAssembly.cxx
  operators/CreateDuct.cxx
  operators/CreateModel.cxx
  operators/CreatePin.cxx
  operators/Delete.cxx
  operators/EditAssembly.cxx
  operators/EditCore.cxx
  operators/EditDuct.cxx
  operators/EditPin.cxx
  )

set (rggHeaders
  Operator.h
  Session.h
  operators/CreateAssembly.h
  operators/CreateDuct.h
  operators/CreateModel.h
  operators/CreatePin.h
  operators/Delete.h
  operators/EditAssembly.h
  operators/EditCore.h
  operators/EditDuct.h
  operators/EditPin.h
  )

add_library(smtkRGGSession ${rggSrcs})

target_link_libraries(smtkRGGSession
  PUBLIC
    smtkCore
  PRIVATE
    ${__dependencies}
)


smtk_export_header(smtkRGGSession Exports.h)

smtk_install_library(smtkRGGSession)

# Operators which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to rggOperatorXML) since the operators themselves include
# the header in their implementations.
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateAssembly.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateDuct.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateModel.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreatePin.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Delete.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditAssembly.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditCore.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditDuct.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditPin.sbt" rggOperatorXML)
smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" rggSessionJSON)

# Install the headers
smtk_public_headers(${rggHeaders})

# TODO
# Add pybind11 and testing
if (SMTK_ENABLE_QT_SUPPORT)
  add_subdirectory(qt)
endif()
if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()
