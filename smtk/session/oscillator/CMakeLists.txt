set(oscillatorSrcs
  Registrar.cxx
  Resource.cxx
  SimulationAttribute.cxx
  operators/CreateModel.cxx
  # operators/Delete.cxx
  operators/EditDomain.cxx
  operators/EditSource.cxx
  operators/Export.cxx
  operators/Read.cxx
  operators/Write.cxx
)

set(oscillatorHeaders
  Registrar.h
  Resource.h
  SimulationAttribute.h
  operators/CreateModel.h
  # operators/Delete.h
  operators/EditDomain.h
  operators/EditSource.h
  operators/Export.h
  operators/Read.h
  operators/Write.h
)

add_library(smtkOscillatorSession ${oscillatorSrcs})
target_link_libraries(smtkOscillatorSession
  PUBLIC
    smtkCore
  PRIVATE
    ${__dependencies}
)
smtk_export_header(smtkOscillatorSession Exports.h)
smtk_install_library(smtkOscillatorSession)

# Operators which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to oscillatorOperatorXML) since the operators themselves include
# the header in their implementations.
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateModel.sbt" oscillatorOperatorXML)
# smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Delete.sbt" oscillatorOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditDomain.sbt" oscillatorOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditSource.sbt" oscillatorOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Export.sbt" oscillatorOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Read.sbt" oscillatorOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Write.sbt" oscillatorOperatorXML)
# This is not an operation but a simulation attribute. However, we can use
# the XML encoding the same way:
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/SimulationAttribute.sbt" oscillatorOperatorXML)

# Install the headers
smtk_public_headers(smtkOscillatorSession ${oscillatorHeaders})

#if (SMTK_ENABLE_QT_SUPPORT)
#  add_subdirectory(qt)
#endif()
if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)

  set(oscillatorPySrcs)

  set(oscillator_pymodulefiles)

  set(oscillator_pyxmlfiles)

  foreach(pyfile ${oscillatorPySrcs})
    get_filename_component(filename ${pyfile} NAME)
    file(RELATIVE_PATH relativedir ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/${pyfile}"
      "${CMAKE_BINARY_DIR}/${relativedir}/${filename}" @ONLY
      )
    list(APPEND oscillator_pymodulefiles "${CMAKE_BINARY_DIR}/${relativedir}/${filename}")
  endforeach()

  foreach(pyfile ${oscillator_pyxmlfiles})
    get_filename_component(filename ${pyfile} NAME)
    file(RELATIVE_PATH relativefile ${PROJECT_BINARY_DIR} ${pyfile})
    configure_file(
      "${pyfile}"
      "${CMAKE_BINARY_DIR}/${relativefile}" @ONLY
      )
    list(APPEND oscillator_pymodulefiles "${CMAKE_BINARY_DIR}/${relativefile}")
  endforeach()

  file(RELATIVE_PATH relativedir ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install(
    FILES ${oscillator_pymodulefiles}
    DESTINATION "${SMTK_PYTHON_MODULEDIR}/${relativedir}"
    )

  # add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
