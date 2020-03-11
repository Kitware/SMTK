set(multiscaleSrcs
  Registrar.cxx
  Resource.cxx
  Session.cxx
)

set(multiscaleHeaders
  Registrar.h
  Resource.h
  Session.h
)

set(multiscaleOperators
  PartitionBoundaries
  Revolve
  )

# Operations which have XML descriptions in separate files
# need to have it encoded as a string in a header.
foreach (operator ${multiscaleOperators})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
  TARGET_OUTPUT targetName)
  list(APPEND multiscaleSrcs operators/${operator}.cxx)
  list(APPEND multiscaleHeaders operators/${operator}.h)
  list(APPEND _multiscaleDependencies ${targetName})
endforeach()

install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/session/multiscale)

add_library(smtkMultiscaleSession ${multiscaleSrcs})
add_dependencies(smtkMultiscaleSession ${_multiscaleDependencies})
target_compile_definitions(smtkMultiscaleSession PRIVATE "AFRL_DIR=\"${AFRL_DIR}\"")
target_compile_definitions(smtkMultiscaleSession PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")

#set smtkMultiscaleSession to publicly link to smtkCore and VTK
target_link_libraries(smtkMultiscaleSession
  LINK_PUBLIC
    smtkCore
    smtkMeshSession
    smtkIOVTK
    VTK::CommonDataModel
    VTK::PythonInterpreter
  LINK_PRIVATE
    VTK::IOXdmf2
    VTK::ImagingCore
    VTK::FiltersGeneral
    VTK::FiltersGeometry
    VTK::FiltersModeling
    ${Boost_LIBRARIES}
)
smtk_export_header(smtkMultiscaleSession Exports.h)

# Add compile definitions and include directories
#set_property(TARGET smtkMultiscaleSession APPEND PROPERTY COMPILE_FLAGS ${MULTISCALE_DEFINES})
#set_property(TARGET smtkMultiscaleSession APPEND PROPERTY INCLUDE_DIRECTORIES ${MULTISCALE_INCLUDE_DIRS})

#install the library and exports the library when used from a build tree
smtk_install_library(smtkMultiscaleSession)

# Install the headers
smtk_public_headers(smtkMultiscaleSession ${multiscaleHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_smtk_plugin(
    smtkMultiscaleSessionPlugin "1.0"
    REGISTRAR smtk::session::multiscale::Registrar
    MANAGERS smtk::operation::Manager smtk::resource::Manager
    LIBRARIES smtkCore smtkMultiscaleSession)
  smtk_install_library(smtkMultiscaleSessionPlugin)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)

set(multiscalePySrcs
  operators/import_from_deform.py
  )

set(multiscale_pymodulefiles)

set(multiscale_pyxmlfiles)

configureStringAsPyVariable(AFRL_DIR
  ${CMAKE_CURRENT_BINARY_DIR}/AFRLDir.py AFRLDir)
list(APPEND multiscale_pyxmlfiles
  ${CMAKE_CURRENT_BINARY_DIR}/AFRLDir.py)

# TODO consider updating to smtk_encode_file() if multiscale is re-enabled.
smtk_pyoperation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/import_from_deform.sbt" multiscale_pyxmlfiles)

foreach(pyfile ${multiscalePySrcs})
  get_filename_component(filename ${pyfile} NAME)
  file(RELATIVE_PATH relativedir ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/${pyfile}"
    "${CMAKE_BINARY_DIR}/${relativedir}/${filename}" @ONLY
    )
  list(APPEND multiscale_pymodulefiles "${CMAKE_BINARY_DIR}/${relativedir}/${filename}")
endforeach()

foreach(pyfile ${multiscale_pyxmlfiles})
  get_filename_component(filename ${pyfile} NAME)
  file(RELATIVE_PATH relativefile ${PROJECT_BINARY_DIR} ${pyfile})
  configure_file(
    "${pyfile}"
    "${CMAKE_BINARY_DIR}/${relativefile}" @ONLY
    )
  list(APPEND multiscale_pymodulefiles "${CMAKE_BINARY_DIR}/${relativefile}")
endforeach()

file(RELATIVE_PATH relativedir ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
install(
  FILES ${multiscale_pymodulefiles}
  DESTINATION "${SMTK_PYTHON_MODULEDIR}/${relativedir}"
  )

  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
