# Only include the VTK use-file in subdirectories
# that require VTK include paths. Otherwise, command
# lines get too long on some platforms.
include("${VTK_USE_FILE}")
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

set(vtkSrcs
  Registrar.cxx
  Resource.cxx
  Session.cxx
  Operation.cxx
  json/jsonResource.cxx
  operators/Export.cxx
  operators/Import.cxx
  operators/LegacyRead.cxx
  operators/Read.cxx
  operators/Write.cxx
)

set(vtkHeaders
  Registrar.h
  Resource.h
  Session.h
  Operation.h
  json/jsonResource.h
  operators/Export.h
  operators/Import.h
  operators/LegacyRead.h
  operators/Read.h
  operators/Write.h
)

install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/session/vtk)

add_library(smtkVTKSession ${vtkSrcs})

#set smtkVTKSession to publicly link to smtkCore and VTK
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

target_link_libraries(smtkVTKSession
  LINK_PUBLIC
    smtkCore
    smtkIOVTK
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
target_include_directories(smtkVTKSession
  PRIVATE
  ${includes_from_vtk}
  )
smtk_export_header(smtkVTKSession Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkVTKSession)

# Operations which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to vtkOperationXML) since the operations themselves include
# the header in their implementations.
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Export.sbt" vtkOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Import.sbt" vtkOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/LegacyRead.sbt" vtkOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Read.sbt" vtkOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Write.sbt" vtkOperationXML)

# Install the headers
smtk_public_headers(${vtkHeaders})

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_smtk_plugin(
    smtkVTKSessionPlugin "1.0"
    REGISTRAR smtk::session::vtk::Registrar
    MANAGERS smtk::operation::Manager smtk::resource::Manager
    LIBRARIES smtkCore smtkVTKSession)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_session_vtk
    LINK_PRIVATE
      vtkIOExodus
      vtkIOParallelExodus
      vtkFiltersGeometry
      vtkCommonDataModel
  )
  add_subdirectory(testing)
endif()
