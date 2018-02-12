# Only include the VTK use-file in subdirectories
# that require VTK include paths. Otherwise, command
# lines get too long on some platforms.
include("${VTK_USE_FILE}")
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

set(multiscaleSrcs
  RegisterSession.cxx
  Resource.cxx
  Session.cxx
  operators/PartitionBoundaries.cxx
  operators/Revolve.cxx
)

set(multiscaleHeaders
  RegisterSession.h
  Resource.h
  Session.h
  operators/PartitionBoundaries.h
  operators/Revolve.h
)

install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/bridge/multiscale)

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
smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" multiscaleSessionJSON)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/PartitionBoundaries.sbt" multiscaleOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Revolve.sbt" multiscaleOperationXML)

# Install the headers
smtk_public_headers(${multiscaleHeaders})

add_subdirectory(environment)

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)

set(multiscalePySrcs
  operators/import_from_deform.py
  )

  set(multiscale_pymodulefiles)

configureStringAsPyVariable(AFRL_DIR
  ${CMAKE_CURRENT_BINARY_DIR}/AFRLDir.py AFRLDir)
list(APPEND multiscale_pymodulefiles
  ${CMAKE_CURRENT_BINARY_DIR}/AFRLDir.py)

  smtk_pyoperation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/import_from_deform.sbt" multiscale_pymodulefiles)

  foreach(pyfile ${multiscalePySrcs})
    get_filename_component(filename ${pyfile} NAME)
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/${pyfile}"
      "${CMAKE_CURRENT_BINARY_DIR}/${filename}" @ONLY
      )
    list(APPEND multiscale_pymodulefiles "${CMAKE_CURRENT_BINARY_DIR}/${filename}")
  endforeach()
  install(
    FILES ${multiscale_pymodulefiles}
    DESTINATION "${SMTK_PYTHON_MODULEDIR}/smtk/bridge/multiscale"
    )

  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_bridge_multiscale
    LINK_PRIVATE
      vtkFiltersGeometry
      vtkCommonDataModel
  )
  add_subdirectory(testing)
endif()
