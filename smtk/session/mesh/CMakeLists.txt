set(meshSrcs
  Facade.cxx
  Registrar.cxx
  Resource.cxx
  Session.cxx
  Topology.cxx
  operators/CreateUniformGrid.cxx
  operators/EulerCharacteristicRatio.cxx
  operators/Export.cxx
  operators/Import.cxx
  operators/Merge.cxx
  operators/Print.cxx
  operators/Read.cxx
  operators/Write.cxx
)

set(meshHeaders
  Facade.h
  Registrar.h
  Resource.h
  Session.h
  Topology.h
  operators/CreateUniformGrid.h
  operators/EulerCharacteristicRatio.h
  operators/Export.h
  operators/Import.h
  operators/Merge.h
  operators/Print.h
  operators/Read.h
  operators/Write.h
)

if (SMTK_ENABLE_VTK_SUPPORT)
  list(APPEND meshSrcs
    vtk/Geometry.cxx)
  list(APPEND meshHeaders
    vtk/Geometry.h)
endif ()

add_library(smtkMeshSession ${meshSrcs})

# Operations which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to meshOperationXML) since the operators themselves include
# the header in their implementations.
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateUniformGrid.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EulerCharacteristicRatio.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Export.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Import.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Merge.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Print.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Read.sbt" meshOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Write.sbt" meshOperationXML)

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
