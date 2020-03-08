set(meshSrcs
  Facade.cxx
  Registrar.cxx
  Resource.cxx
  Session.cxx
  Topology.cxx
)

set(meshHeaders
  Facade.h
  Registrar.h
  Resource.h
  Session.h
  Topology.h
)

if (SMTK_ENABLE_VTK_SUPPORT)
  list(APPEND meshSrcs
    vtk/Geometry.cxx)
  list(APPEND meshHeaders
    vtk/Geometry.h)
endif ()

set(meshOperators
  CreateUniformGrid
  EulerCharacteristicRatio
  Export
  Import
  Merge
  Print
  Read
  Write
  )

# Operations which have XML descriptions in separate files
# need to have it encoded as a string in a header.
foreach (operator ${meshOperators})
  smtk_operation_xml_new("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
  TARGET_OUTPUT targetName)
  list(APPEND meshSrcs operators/${operator}.cxx)
  list(APPEND meshHeaders operators/${operator}.h)
  list(APPEND meshDependencies ${targetName})
endforeach()

add_library(smtkMeshSession ${meshSrcs})
add_dependencies(smtkMeshSession ${meshDependencies})

# Install the headers
smtk_public_headers(smtkMeshSession ${meshHeaders})

set(vtk_dependencies)

if (SMTK_ENABLE_VTK_SUPPORT)
  target_compile_definitions(smtkMeshSession PUBLIC VTK_SUPPORT)

  list(APPEND vtk_dependencies
    VTK::CommonCore
    VTK::CommonDataModel
    vtkSMTKSourceExt
    )
endif ()

target_link_libraries(smtkMeshSession
  LINK_PUBLIC
    smtkCore
  LINK_PRIVATE
    ${vtk_dependencies}
    ${__dependencies}
)

smtk_export_header(smtkMeshSession Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkMeshSession)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
add_subdirectory(testing)
endif()
