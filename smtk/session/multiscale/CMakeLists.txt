# Only include the VTK use-file in subdirectories
# that require VTK include paths. Otherwise, command
# lines get too long on some platforms.
include("${VTK_USE_FILE}")
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

set(multiscaleSrcs
  Registrar.cxx
  Resource.cxx
  Session.cxx
  operators/PartitionBoundaries.cxx
  operators/Revolve.cxx
)

set(multiscaleHeaders
  Registrar.h
  Resource.h
  Session.h
  operators/PartitionBoundaries.h
  operators/Revolve.h
)

install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/session/multiscale)

add_library(smtkMultiscaleSession ${multiscaleSrcs})
target_compile_definitions(smtkMultiscaleSession PRIVATE "AFRL_DIR=\"${AFRL_DIR}\"")
target_compile_definitions(smtkMultiscaleSession PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")

#set smtkMultiscaleSession to publicly link to smtkCore and VTK
target_link_libraries(smtkMultiscaleSession
  LINK_PUBLIC
    smtkCore
    smtkMeshSession
    smtkIOVTK
    vtkCommonDataModel
    vtkPythonInterpreter
  LINK_PRIVATE
    vtkIOXdmf2
    vtkImagingCore
    vtkFiltersGeneral
    vtkFiltersGeometry
    vtkFiltersModeling
    ${Boost_LIBRARIES}
)
smtk_export_header(smtkMultiscaleSession Exports.h)

# Add compile definitions and include directories
#set_property(TARGET smtkMultiscaleSession APPEND PROPERTY COMPILE_FLAGS ${MULTISCALE_DEFINES})
#set_property(TARGET smtkMultiscaleSession APPEND PROPERTY INCLUDE_DIRECTORIES ${MULTISCALE_INCLUDE_DIRS})

#install the library and exports the library when used from a build tree
smtk_install_library(smtkMultiscaleSession)

# Operations which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to multiscaleOperationXML) since the operators themselves include
# the header in their implementations.
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/PartitionBoundaries.sbt" multiscaleOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Revolve.sbt" multiscaleOperationXML)

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

set(matplotlib_pyxmlfiles)

configureStringAsPyVariable(AFRL_DIR
  ${CMAKE_CURRENT_BINARY_DIR}/AFRLDir.py AFRLDir)
list(APPEND multiscale_pyxmlfiles
  ${CMAKE_CURRENT_BINARY_DIR}/AFRLDir.py)

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
