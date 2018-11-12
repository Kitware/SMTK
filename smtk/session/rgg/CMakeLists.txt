set(rggSrcs
  Material.cxx
  Registrar.cxx
  Resource.cxx
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
  operators/ReadRXFFile.cxx
  operators/ReadRXFFileHelper.cxx
  operators/RemoveMaterial.cxx
  )

set(rggHeaders
  Material.h
  Registrar.h
  Resource.h
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
  operators/ReadRXFFile.h
  operators/ReadRXFFileHelper.h
  operators/RemoveMaterial.h
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
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/AddMaterial.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateAssembly.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateDuct.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateModel.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreatePin.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Delete.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditAssembly.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditCore.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditDuct.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditMaterial.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditPin.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ReadRXFFile.sbt" rggOperatorXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/RemoveMaterial.sbt" rggOperatorXML)

# Install the headers
smtk_public_headers(smtkRGGSession ${rggHeaders})

if (SMTK_ENABLE_QT_SUPPORT)
  add_subdirectory(qt)
endif()
if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

# Test if PyArc is available. This requires that (a) we build with python
# bindings enabled and that (b) PyArc be in our python module path.
set(ENABLE_PYARC_BINDINGS FALSE)

if (SMTK_ENABLE_PYTHON_WRAPPING)

  # Test for PyArc module
  execute_process(
    COMMAND
    ${PYTHON_EXECUTABLE}
    -c "try:\n  import PyARC\n  print(\"TRUE\")\nexcept Exception:\n  print(\"FALSE\")"
    RESULT_VARIABLE rv
    OUTPUT_VARIABLE ENABLE_PYARC_BINDINGS
    ERROR_VARIABLE err
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
    )

  if (NOT rv EQUAL 0)
    message(FATAL_ERROR "Could not query for PyArc; return value was ${rv} and error was ${err}.")
  endif()

  set(rggPySrcs)

  set(rgg_pymodulefiles)

  set(rgg_pyxmlfiles)

  if (ENABLE_PYARC_BINDINGS)
    target_compile_definitions(smtkRGGSession PRIVATE "ENABLE_PYARC_BINDINGS")

    list(APPEND rggPySrcs
      operators/export_to_pyarc.py
      )

    smtk_pyoperation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/export_to_pyarc.sbt" rgg_pyxmlfiles)
  endif()

  foreach(pyfile ${rggPySrcs})
    get_filename_component(filename ${pyfile} NAME)
    file(RELATIVE_PATH relativedir ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/${pyfile}"
      "${CMAKE_BINARY_DIR}/${relativedir}/${filename}" @ONLY
      )
    list(APPEND rgg_pymodulefiles "${CMAKE_BINARY_DIR}/${relativedir}/${filename}")
  endforeach()

  foreach(pyfile ${rgg_pyxmlfiles})
    get_filename_component(filename ${pyfile} NAME)
    file(RELATIVE_PATH relativefile ${PROJECT_BINARY_DIR} ${pyfile})
    configure_file(
      "${pyfile}"
      "${CMAKE_BINARY_DIR}/${relativefile}" @ONLY
      )
    list(APPEND rgg_pymodulefiles "${CMAKE_BINARY_DIR}/${relativefile}")
  endforeach()

  file(RELATIVE_PATH relativedir ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install(
    FILES ${rgg_pymodulefiles}
    DESTINATION "${SMTK_PYTHON_MODULEDIR}/${relativedir}"
    )

  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
