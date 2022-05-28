set(oscillatorSrcs
  Registrar.cxx
  Resource.cxx
  SimulationAttribute.cxx
)

set(oscillatorHeaders
  Registrar.h
  Resource.h
  SimulationAttribute.h
)

set(oscillatorOperators
  CreateModel
  # Delete
  EditDomain
  EditSource
  Export
  Read
  Write
  )

# Operators which have XML descriptions in separate files
# need to have it encoded as a string in a header.
foreach (operator ${oscillatorOperators})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
  HEADER_OUTPUT headerName)
  list(APPEND oscillatorSrcs operators/${operator}.cxx)
  list(APPEND oscillatorHeaders operators/${operator}.h)
  list(APPEND _oscillatorDependencies ${headerName})
endforeach()
# This is not an operation but a simulation attribute. However, we can use
# the XML encoding the same way:
smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/SimulationAttribute.sbt"
  HEADER_OUTPUT headerName)
list(APPEND _oscillatorDependencies ${headerName})
add_custom_target(oscillatorSessionGenHeaders DEPENDS ${_oscillatorDependencies})

add_library(smtkOscillatorSession ${oscillatorSrcs})
add_dependencies(smtkOscillatorSession oscillatorSessionGenHeaders)
target_link_libraries(smtkOscillatorSession
  PUBLIC
    smtkCore
  PRIVATE
    ${__dependencies}
    Boost::boost
)
smtk_export_header(smtkOscillatorSession Exports.h)
smtk_install_library(smtkOscillatorSession)

# Install the headers
smtk_public_headers(smtkOscillatorSession ${oscillatorHeaders})

#if (SMTK_ENABLE_QT_SUPPORT)
#  add_subdirectory(qt)
#endif()
if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
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
