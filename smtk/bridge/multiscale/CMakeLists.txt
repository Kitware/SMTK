# Only include the VTK use-file in subdirectories
# that require VTK include paths. Otherwise, command
# lines get too long on some platforms.
include("${VTK_USE_FILE}")
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

set(multiscaleSrcs
  Session.cxx
  Operator.cxx
  operators/PartitionBoundaries.cxx
  operators/Dream3DPipeline.cxx
  operators/PythonScript.cxx
  operators/Revolve.cxx
)

set(multiscaleHeaders
  Session.h
  Operator.h
  operators/PartitionBoundaries.h
  operators/Dream3DPipeline.h
  operators/PythonScript.h
  operators/Revolve.h
)

install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/bridge/multiscale)

add_library(smtkMultiscaleSession ${multiscaleSrcs})
set_target_properties(smtkMultiscaleSession PROPERTIES COMPILE_FLAGS "-DAFRL_DIR=${AFRL_DIR} -DVTKPY_DIR=${VTKPY_DIR}")
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

# Operators which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to multiscaleOperatorXML) since the operators themselves include
# the header in their implementations.
smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" multiscaleSessionJSON)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/PartitionBoundaries.sbt" multiscaleOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Dream3DPipeline.sbt" multiscaleOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Revolve.sbt" multiscaleOperatorXML)

# Install the headers
smtk_public_headers(${multiscaleHeaders})

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)
  #extract the headers from multiscale library we built to give them to shiboken

  sbk_wrap_library(smtkMultiscaleSession
    PACKAGE smtk
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${SMTK_SOURCE_DIR}/smtk
    LOCAL_INCLUDE_DIRECTORIES
      ${SMTK_SOURCE_DIR}/smtk/common
      ${SMTK_SOURCE_DIR}/smtk/attribute
      ${SMTK_SOURCE_DIR}/smtk/model
      ${SMTK_SOURCE_DIR}/smtk/session
      ${SMTK_SOURCE_DIR}/smtk/bridge/multiscale
      ${SMTK_SOURCE_DIR}/smtk/simulation
      ${SMTK_SOURCE_DIR}/smtk/io
      ${SMTK_SOURCE_DIR}/smtk/view
      ${SMTK_SOURCE_DIR}/smtk
      ${SMTK_BINARY_DIR}/smtk
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/typesystem.xml
    HEADERS ${multiscaleHeaders}
    DEPENDS smtkCore
  )
target_include_directories(smtkMultiscaleSessionPython
    PRIVATE ${MULTISCALE_INCLUDE_DIRS})
endif()

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

if (SMTK_USE_PYBIND11)
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
