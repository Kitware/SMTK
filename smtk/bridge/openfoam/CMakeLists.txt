# Only include the VTK use-file in subdirectories
# that require VTK include paths. Otherwise, command
# lines get too long on some platforms.
include("${VTK_USE_FILE}")
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

set(openfoamSrcs
  Session.cxx
  Operator.cxx
)

set(openfoamHeaders
  Session.h
  Operator.h
)

install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/bridge/openfoam)

add_library(smtkOpenFOAMSession ${openfoamSrcs})
target_compile_definitions(smtkOpenFOAMSession PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")

#set smtkOpenfoamSession to publicly link to smtkCore and VTK
target_link_libraries(smtkOpenFOAMSession
  LINK_PUBLIC
    smtkCore
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
smtk_export_header(smtkOpenFOAMSession Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkOpenFOAMSession)

# Operators which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to openfoamOperatorXML) since the operators themselves include
# the header in their implementations.
smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" openfoamSessionJSON)

# Install the headers
smtk_public_headers(${openfoamHeaders})

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)

set(openfoamPySrcs
  operators/add_obstacle.py
  operators/constant_input_files.py
  operators/create_wind_tunnel.py
  operators/set_main_controls.py
  operators/set_working_directory.py
  )

  set(openfoam_pymodulefiles)

  smtk_pyoperator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/add_obstacle.sbt" openfoam_pymodulefiles)
  smtk_pyoperator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/create_wind_tunnel.sbt" openfoam_pymodulefiles)
  smtk_pyoperator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/set_main_controls.sbt" openfoam_pymodulefiles)
  smtk_pyoperator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/set_working_directory.sbt" openfoam_pymodulefiles)

  foreach(pyfile ${openfoamPySrcs})
    get_filename_component(filename ${pyfile} NAME)
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/${pyfile}"
      "${CMAKE_CURRENT_BINARY_DIR}/${filename}" @ONLY
      )
    list(APPEND openfoam_pymodulefiles "${CMAKE_CURRENT_BINARY_DIR}/${filename}")
  endforeach()
  install(
    FILES ${openfoam_pymodulefiles}
    DESTINATION "${SMTK_PYTHON_MODULEDIR}/smtk/bridge/openfoam"
    )

  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_bridge_openfoam
    LINK_PRIVATE
      vtkFiltersGeometry
      vtkCommonDataModel
  )
  add_subdirectory(testing)
endif()
