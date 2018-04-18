set (rggSrcs
  Material.cxx
  Operator.cxx
  Session.cxx
  operators/AddMaterial.cxx
  operators/CreateAssembly.cxx
  operators/CreateDuct.cxx
  operators/CreateModel.cxx
  operators/CreatePin.cxx
  operators/Delete.cxx
  operators/EditAssembly.cxx
  operators/EditCore.cxx
  operators/EditDuct.cxx
  operators/EditMaterial.cxx
  operators/EditPin.cxx
  operators/RemoveMaterial.cxx
  operators/ReadRXFFile.cxx
  operators/ReadRXFFileHelper.cxx
  )

set (rggHeaders
  Material.h
  Operator.h
  Session.h
  operators/AddMaterial.h
  operators/CreateAssembly.h
  operators/CreateDuct.h
  operators/CreateModel.h
  operators/CreatePin.h
  operators/Delete.h
  operators/EditAssembly.h
  operators/EditCore.h
  operators/EditDuct.h
  operators/EditMaterial.h
  operators/EditPin.h
  operators/RemoveMaterial.h
  operators/ReadRXFFile.h
  operators/ReadRXFFileHelper.h
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
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/AddMaterial.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateAssembly.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateDuct.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateModel.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreatePin.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Delete.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditAssembly.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditCore.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditDuct.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditMaterial.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditPin.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/RemoveMaterial.sbt" rggOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ReadRXFFile.sbt" rggOperatorXML)
smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" rggSessionJSON)

# Install the headers
smtk_public_headers(${rggHeaders})

if (SMTK_ENABLE_QT_SUPPORT)
  add_subdirectory(qt)
endif()
if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)

set(rggPySrcs
  operators/export_to_pyarc.py
  operators/export_to_pyarcHelper.py
  )

  set(rgg_pymodulefiles)
  smtk_pyoperator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/export_to_pyarc.sbt"
    rgg_pymodulefiles)

  foreach(pyfile ${rggPySrcs})
    get_filename_component(filename ${pyfile} NAME)
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/${pyfile}"
      "${CMAKE_CURRENT_BINARY_DIR}/${filename}" @ONLY
      )
    list(APPEND rgg_pymodulefiles "${CMAKE_CURRENT_BINARY_DIR}/${filename}")
  endforeach()
  install(
    FILES ${rgg_pymodulefiles}
    DESTINATION "${SMTK_PYTHON_MODULEDIR}/smtk/bridge/rgg"
    )
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
