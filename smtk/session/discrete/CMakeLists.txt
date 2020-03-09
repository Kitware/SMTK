include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/operators"
    "${CMAKE_CURRENT_SOURCE_DIR}/operation"
    "${CMAKE_CURRENT_BINARY_DIR}/operation"
    "${CMAKE_CURRENT_BINARY_DIR}/extension"
)

# Build vtk moab reader before session
if(SMTK_ENABLE_MOAB_DISCRETE_READER)
  add_subdirectory(moabreader)
endif()

SET(DiscreteModelFiles
  operation/vtkCMBMapToCMBModel.cxx
  operation/vtkCMBModelBuilder.cxx
  operation/vtkCMBModelWriterV2.cxx
  operation/vtkCMBModelWriterV4.cxx
  operation/vtkCMBModelWriterV5.cxx
  operation/vtkCMBModelWriterBase.cxx
  operation/vtkCMBParserBase.cxx
  operation/vtkCMBParserV2.cxx
  operation/vtkCMBParserV4.cxx
  operation/vtkCMBParserV5.cxx
  operation/vtkCMBModelReader.cxx
)

SET(DiscreteOperationFiles
  operation/vtkCMBModelPointsOperation.cxx
  operation/vtkEdgeSplitOperation.cxx
  operation/vtkEdgeSplitOperationBase.cxx
  operation/vtkMergeOperation.cxx
  operation/vtkMergeOperationBase.cxx
  operation/vtkCMBModelReadOperation.cxx
  operation/vtkCreateModelEdgesOperation.cxx
  operation/vtkCreateModelEdgesOperationBase.cxx
  operation/vtkSplitOperation.cxx
  operation/vtkSplitOperationBase.cxx
  operation/vtkMasterPolyDataNormals.cxx
  operation/vtkMaterialOperation.cxx
  operation/vtkMaterialOperationBase.cxx
  operation/vtkMergeDuplicateCells.cxx
  operation/vtkModelEntityGroupOperation.cxx
  operation/vtkModelEntityGroupOperationBase.cxx
  operation/vtkModelEntityOperationBase.cxx
  operation/vtkGenerateSimpleModelOperation.cxx
  operation/vtkSeedGrowSelectionFilter.cxx
  operation/vtkSelectionSplitOperation.cxx
  operation/vtkSelectionSplitOperationBase.cxx
  operation/vtkCompleteShells.cxx
  operation/vtkCMBIncorporateMeshOperation.cxx
  operation/vtkEnclosingModelEntityOperation.cxx
)

SET(DiscreteModelHeaders
  operation/vtkCMBModelPointsOperation.h
  operation/vtkCMBMapToCMBModel.h
  operation/vtkCMBModelBuilder.h
  operation/vtkCMBModelWriterV2.h
  operation/vtkCMBModelWriterV4.h
  operation/vtkCMBModelWriterV5.h
  operation/vtkCMBModelWriterBase.h
  operation/vtkCMBParserBase.h
  operation/vtkCMBParserV2.h
  operation/vtkCMBParserV4.h
  operation/vtkCMBParserV5.h
  operation/vtkCMBModelReader.h

  operation/vtkEdgeSplitOperation.h
  operation/vtkEdgeSplitOperationBase.h
  operation/vtkMergeOperation.h
  operation/vtkMergeOperationBase.h
  operation/vtkCMBModelReadOperation.h
  operation/vtkCreateModelEdgesOperation.h
  operation/vtkCreateModelEdgesOperationBase.h
  operation/vtkSplitOperation.h
  operation/vtkSplitOperationBase.h
  operation/vtkMasterPolyDataNormals.h
  operation/vtkMaterialOperation.h
  operation/vtkMaterialOperationBase.h
  operation/vtkMergeDuplicateCells.h
  operation/vtkModelEntityGroupOperation.h
  operation/vtkModelEntityGroupOperationBase.h
  operation/vtkModelEntityOperationBase.h
  operation/vtkGenerateSimpleModelOperation.h
  operation/vtkSeedGrowSelectionFilter.h
  operation/vtkSelectionSplitOperation.h
  operation/vtkSelectionSplitOperationBase.h
  operation/vtkCompleteShells.h
  operation/vtkCMBIncorporateMeshOperation.h
  operation/vtkEnclosingModelEntityOperation.h
)


SET_SOURCE_FILES_PROPERTIES(
  operation/vtkCMBParserBase
  ABSTRACT
)

SET_SOURCE_FILES_PROPERTIES(
  ${DiscreteModelFiles}
  PROPERTIES
    WRAP_EXCLUDE
    WRAP_EXCLUDE_PYTHON
)

set(_module_src
  ${DiscreteModelFiles}
  ${DiscreteOperationFiles}
)

set(discreteSessionSrcs
  Registrar.cxx
  Resource.cxx
  Session.cxx
  Operation.cxx
  ArrangementHelper.cxx
)

set(discreteSessionHeaders
  Registrar.h
  Resource.h
  Session.h
  Operation.h
  ArrangementHelper.h
)

set(discreteSessionOperators
  CreateEdgesOperation
  EntityGroupOperation
  ImportOperation
  LegacyReadResource
  MergeOperation
  ReadOperation
  ReadResource
  RemoveModel
  SetProperty
  SplitFaceOperation
  WriteOperation
  WriteResource
  # TODO: These operators use the old MeshSelectionItem
  # and need to be updated in order to be used
  # EdgeOperation
  # GrowOperation
  )

include(SMTKOperationXML)

# construct operator inputs
foreach (operator ${discreteSessionOperators})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
  TARGET_OUTPUT targetName)
  list(APPEND discreteSessionSrcs operators/${operator}.cxx)
  list(APPEND discreteSessionHeaders operators/${operator}.h)
  list(APPEND _discreteSessionDependencies ${targetName})
endforeach()

add_library(smtkDiscreteSession ${_module_src} ${discreteSessionSrcs})
add_dependencies(smtkDiscreteSession ${_discreteSessionDependencies})

set (__public_dependencies)
set (__private_dependencies)

if (VTK_WRAP_PYTHON)
  list(APPEND __public_dependencies VTK::PythonUsed)
  list(APPEND __private_dependencies VTK::PythonInterpreter)
endif (VTK_WRAP_PYTHON)

if(SMTK_ENABLE_MOAB_DISCRETE_READER)
  list(APPEND __private_dependencies vtkDiscreteMoabReader)
endif()

vtk_module_scan(
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/kernel/vtk.module"
  PROVIDES_MODULES vtk_modules
  HIDE_MODULES_FROM_CACHE ON
  WANT_BY_DEFAULT ON)
vtk_module_build(
  MODULES ${vtk_modules}
  PACKAGE SMTKDiscreteModules
  INSTALL_EXPORT SMTKDiscreteModules
  CMAKE_DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
  HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${PROJECT_VERSION}"
  TEST_DIRECTORY_NAME "NONE")

#set smtkDiscreteSession to publicly link to smtkCore and privately to vtkSMTKDiscreteModel
target_link_libraries(smtkDiscreteSession
  LINK_PUBLIC
    smtkCore
    smtkIOVTK
    ${__public_dependencies}
  LINK_PRIVATE
  ${Boost_LIBRARIES}
    VTK::CommonExecutionModel
    VTK::FiltersCore
    VTK::FiltersGeometry
    VTK::FiltersSources
    VTK::GeovisCore
    VTK::IOGDAL
    VTK::IOXML
    VTK::IOParallel
    VTK::RenderingCore
    VTK::RenderingFreeType
    VTK::RenderingGL2PSOpenGL2
    VTK::RenderingContextOpenGL2
    VTK::RenderingOpenGL2
    VTK::vtksys
    ${__private_dependencies}
    vtkSMTKReaderExt
    vtkSMTKDiscreteModel
    vtkSMTKFilterExt
  )

if (SMTK_ENABLE_REMUS_SUPPORT)
  target_link_libraries(smtkDiscreteSession
    LINK_PRIVATE
      vtkSMTKMeshingExt)
endif ()


smtk_export_header(smtkDiscreteSession Exports.h)

# Install the headers.
smtk_public_headers(smtkDiscreteSession ${discreteSessionHeaders} ${DiscreteModelHeaders})

#install the library and exports the library when used from a build tree
set(PROJECT_NAME SMTKDiscreteModules)
smtk_install_library(smtkDiscreteSession)

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
  add_subdirectory(operation/testing)
endif()
