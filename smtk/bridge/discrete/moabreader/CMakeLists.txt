project(DiscreteMoabReader)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/detail"
    "${CMAKE_CURRENT_SOURCE_DIR}/model"
    "${CMAKE_CURRENT_BINARY_DIR}/detail"
    "${CMAKE_CURRENT_BINARY_DIR}/model"
)

set(headers
  CellSets.h
  DataSetConverter.h
  detail/CellTypeToType.h
  detail/ContinousCellInfo.h
  detail/LoadGeometry.h
  detail/MixedCellConnectivity.h
  detail/ReadSparseTag.h
  detail/ReduceSpectralMesh.h
  detail/UsageTable.h
  ExtractShell.h
  model/BoundaryConditions.h
  model/FaceSets.h
  SimpleMoab.h
  vtkCmbMoabReader.h
  )

add_library(vtkDiscreteMoabReader vtkCmbMoabReader.cxx)

#link agianst the discrete Model library and moab
target_link_libraries(vtkDiscreteMoabReader
                      LINK_PUBLIC
                      MOAB
                      vtkSMTKDiscreteModel
                      )



smtk_export_header(vtkDiscreteMoabReader discreteMoabExports.h)

# Install the headers.
smtk_public_headers(vtkCmbMoabReader.h)

#install the library and exports the library when used from a build tree
smtk_install_library(vtkDiscreteMoabReader DEPENDS vtkSMTKDiscreteModel)

target_include_directories(vtkDiscreteMoabReader
                          PUBLIC ${MOAB_INCLUDE_DIRS})
