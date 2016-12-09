# Only include the VTK use-file in subdirectories
# that require VTK include paths. Otherwise, command
# lines get too long on some platforms.
include("${VTK_USE_FILE}")
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

set(exodusSrcs
  Session.cxx
  SessionExodusIOJSON.cxx
  Operator.cxx
  ReadOperator.cxx
  WriteOperator.cxx
)

set(exodusHeaders
  Session.h
  SessionExodusIOJSON.h
  Operator.h
  ReadOperator.h
  WriteOperator.h
)

install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/bridge/exodus)

add_library(smtkExodusSession ${exodusSrcs})

#set smtkExodusSession to publicly link to smtkCore and VTK
set(__dependencies)
foreach(lib IN ITEMS
    vtkIOMPIImage
    vtkIOMPIParallel
    )
  if (TARGET ${lib})
    list(APPEND __dependencies ${lib})
  endif()
endforeach()

set(import_vtk_depends
    vtkIOExodus
    vtkIONetCDF
    vtkIOXML
    vtkImagingCore
    vtkFiltersGeneral
    vtkIOLegacy
    vtkIOParallel
    vtkIOParallelExodus
    vtkIOGeometry
    vtkFiltersGeometry
    )

target_link_libraries(smtkExodusSession
  LINK_PUBLIC
    smtkCore
    vtkCommonDataModel
  LINK_PRIVATE
    ${import_vtk_depends}
    ${__dependencies}
    ${Boost_LIBRARIES}
)

#vtk targets don't specify an include directory through usage-requirements, so
#we manually add them here
set(includes_from_vtk)
foreach(module ${import_vtk_depends})
  list(APPEND includes_from_vtk "${${module}_INCLUDE_DIRS}")
endforeach(module)
target_include_directories(smtkExodusSession
  PRIVATE
  ${includes_from_vtk}
  )
smtk_export_header(smtkExodusSession Exports.h)

# Add compile definitions and include directories
#set_property(TARGET smtkExodusSession APPEND PROPERTY COMPILE_FLAGS ${EXODUS_DEFINES})
#set_property(TARGET smtkExodusSession APPEND PROPERTY INCLUDE_DIRECTORIES ${EXODUS_INCLUDE_DIRS})

#install the library and exports the library when used from a build tree
smtk_install_library(smtkExodusSession)

# Operators which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to exodusOperatorXML) since the operators themselves include
# the header in their implementations.
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/ReadOperator.sbt" exodusOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/WriteOperator.sbt" exodusOperatorXML)
smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" exodusSessionJSON)

# Install the headers
smtk_public_headers(${exodusHeaders})

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)
  #extract the headers from exodus library we built to give them to shiboken

  sbk_wrap_library(smtkExodusSession
    PACKAGE smtk
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${SMTK_SOURCE_DIR}/smtk
    LOCAL_INCLUDE_DIRECTORIES
      ${SMTK_SOURCE_DIR}/smtk/common
      ${SMTK_SOURCE_DIR}/smtk/attribute
      ${SMTK_SOURCE_DIR}/smtk/model
      ${SMTK_SOURCE_DIR}/smtk/session
      ${SMTK_SOURCE_DIR}/smtk/bridge/exodus
      ${SMTK_SOURCE_DIR}/smtk/simulation
      ${SMTK_SOURCE_DIR}/smtk/io
      ${SMTK_SOURCE_DIR}/smtk/view
      ${SMTK_SOURCE_DIR}/smtk
      ${SMTK_BINARY_DIR}/smtk
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/typesystem.xml
    HEADERS ${exodusHeaders}
    DEPENDS smtkCore
  )
target_include_directories(smtkExodusSessionPython
    PRIVATE ${EXODUS_INCLUDE_DIRS})
endif()

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

if (SMTK_USE_PYBIND11)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_bridge_exodus
    LINK_PRIVATE
      vtkIOExodus
      vtkIOParallelExodus
      vtkFiltersGeometry
      vtkCommonDataModel
  )
  add_subdirectory(testing)
endif()
