set(vtkSrcs
  Geometry.cxx
  Registrar.cxx
  Resource.cxx
  Session.cxx
  Operation.cxx
  json/jsonResource.cxx
)

set(vtkHeaders
  Geometry.h
  RegisterVTKBackend.h
  Registrar.h
  Resource.h
  Session.h
  Operation.h
  json/jsonResource.h
)

set(vtkOperators
  Export
  Import
  LegacyRead
  Read
  Write
  )

# Operations which have XML descriptions in separate files
# need to have it encoded as a string in a header.
foreach (operator ${vtkOperators})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
  HEADER_OUTPUT headerName)
  list(APPEND vtkSrcs operators/${operator}.cxx)
  list(APPEND vtkHeaders operators/${operator}.h)
  list(APPEND _vtkDependencies ${headerName})
endforeach()
add_custom_target(vtkSessionGenHeaders DEPENDS ${_vtkDependencies})

install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/session/vtk)

add_library(smtkVTKSession ${vtkSrcs})
add_dependencies(smtkVTKSession vtkSessionGenHeaders)

#set smtkVTKSession to publicly link to smtkCore and VTK
set(__dependencies)
foreach(lib IN ITEMS
    VTK::IOMPIImage
    VTK::IOMPIParallel
    )
  if (TARGET ${lib})
    list(APPEND __dependencies ${lib})
  endif()
endforeach()

set(import_vtk_depends
  VTK::IOExodus
  VTK::IONetCDF
  VTK::IOXML
  VTK::ImagingCore
  VTK::FiltersGeneral
  VTK::IOLegacy
  VTK::IOParallel
  VTK::IOParallelExodus
  VTK::IOGeometry
  VTK::FiltersGeometry
)

target_link_libraries(smtkVTKSession
  LINK_PUBLIC
    smtkCore
    smtkIOVTK
    vtkSMTKSourceExt
    vtkSMTKModelExt
    VTK::CommonDataModel
  LINK_PRIVATE
    ${import_vtk_depends}
    ${__dependencies}
    ${Boost_LIBRARIES}
)
vtk_module_autoinit(
  TARGETS smtkVTKSession
  MODULES VTK::CommonDataModel
          ${import_vtk_depends})

smtk_export_header(smtkVTKSession Exports.h)

#install the library and exports the library when used from a build tree
set(SMTK_EXPORT_SET SMTKVTKSession)
smtk_install_library(smtkVTKSession)

install(
  EXPORT ${SMTK_EXPORT_SET}
  DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
  FILE ${SMTK_EXPORT_SET}-targets.cmake
)
export(EXPORT ${SMTK_EXPORT_SET} FILE "${PROJECT_BINARY_DIR}/${SMTK_INSTALL_CONFIG_DIR}/${SMTK_EXPORT_SET}-targets.cmake")

# Install the headers
smtk_public_headers(smtkVTKSession ${vtkHeaders})

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
