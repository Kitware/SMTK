project(CMBModel)

find_package(SMTK REQUIRED)
find_package(Remus REQUIRED)

set(CMAKE_MODULE_PATH "${SMTK_ROOT_DIR}/lib/cmake/SMTK" ${CMAKE_MODULE_PATH})
include(SMTKOperationXML)

SET(CmbModelFiles
  vtkCMBModelWriterV2.cxx
  vtkCMBModelWriterV4.cxx
  vtkCMBModelWriterV5.cxx
  vtkCMBParserBase.cxx
  vtkCMBParserV2.cxx
  vtkCMBParserV4.cxx
  vtkCMBParserV5.cxx
  vtkCMBModelOmicronMeshInputWriter.cxx
  vtkEnclosingModelEntityOperation.cxx
  vtkCMBModelActor.cxx
)

SET(CMBOperationFiles
  vtkADHExporterOperation.cxx
  vtkADHExporterOperationBase.cxx
  vtkAppendSolids.cxx
  vtkMaterialOperation.cxx
  vtkMaterialOperationBase.cxx
  vtkMergeOperation.cxx
  vtkMergeOperationBase.cxx
  vtkCMBImportBCFileOperation.cxx
  vtkCMBIncorporateMeshOperation.cxx
  vtkCMBModelBuilder.cxx
  vtkCMBMapToCMBModel.cxx
  vtkCMBModelReader.cxx
  vtkCMBModelReadOperation.cxx
  vtkCMBModelWriterBase.cxx
  vtkCMBPolyDataProvider.cxx
  vtkCreateModelEdgesOperation.cxx
  vtkCreateModelEdgesOperationBase.cxx
  vtkGenerateSimpleModelOperation.cxx
  vtkModelEntityGroupOperation.cxx
  vtkModelEntityGroupOperationBase.cxx
  vtkModelEntityOperation.cxx
  vtkModelEntityOperationBase.cxx
  vtkSeedGrowSelectionFilter.cxx
  vtkSelectionSplitOperation.cxx
  vtkSelectionSplitOperationBase.cxx
  vtkSplitOperation.cxx
  vtkSplitOperationBase.cxx
  vtkCMBModelStateOperationBase.cxx
  vtkCMBModelStateOperation.cxx
  vtkCMBXMLBCSWriter.cxx
  vtkCMBModelOmicronMeshInputWriterBase.cxx
  vtkMergeDuplicateCells.cxx
  vtkMasterPolyDataNormals.cxx
  vtkCompleteShells.cxx
  vtkModelEdgeOperationBase.cxx
  vtkModelEdgeOperation.cxx
  vtkEdgeSplitOperation.cxx
  vtkEdgeSplitOperationBase.cxx
  vtkGeoTransformOperation.cxx
  vtkCMBModelPointsOperation.cxx
# This class should really be in ${Plugin_Src_Files},
# but it is linked from SimBuilderMeshPlugin
  vtkSMOperationProxy.cxx
  vtkCMBModelRepresentation.cxx
  vtkCMBModelSource.cxx
  vtkCMBModelMapper.cxx
  vtkCMBModelSelectionSource.cxx
  vtkCMBModelSelectionPainter.cxx
  #vtkCmbModelDeliveryFilter.cxx
  #vtkCmbModelSelectionConverter.cxx
  vtkCMBMeshServerJobSubmitter.cxx
)

if(PARAVIEW_ENABLE_PYTHON)
  list(APPEND CMBOperationFiles vtkPythonExporter.cxx)
  list(APPEND CmbModelFiles
    PythonExportGridInfo.cxx
    PythonExportGridInfo2D.cxx
    PythonExportGridInfo3D.cxx
  )
else()
  message(STATUS "ParaView should be built with Python enabled in order to export from SimBuilder.")
endif()

SET_SOURCE_FILES_PROPERTIES(
  vtkCMBParserBase
  ABSTRACT
)

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/CreateEdgesOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/MergeOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/ReadOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/SplitFaceOperation.sbt" unitOperationXML)

# Disable manifests for plugins on Windows so that they do not need to resolve their dependencies.
if (MSVC)
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif(MSVC)

SET_SOURCE_FILES_PROPERTIES(
  ${CmbModelFiles}
  PROPERTIES
    WRAP_EXCLUDE
    WRAP_EXCLUDE_PYTHON
)

set(_module_src
  ${CmbModelFiles}
  ${CMBOperationFiles}
)
set(vtkCmbDiscreteModel_NO_HeaderTest 1)
set(VTK_INSTALL_NO_HEADERS 1)
vtk_module_library(vtkCmbDiscreteModel ${_module_src})
target_include_directories(${vtk-module}
  PUBLIC
    $<BUILD_INTERFACE:${smtk_SOURCE_DIR}>
    $<BUILD_INTERFACE:${smtk_BINARY_DIR}>
    $<INSTALL_INTERFACE:include/smtk/${SMTK_VERSION}>)

target_link_libraries(vtkCmbDiscreteModel
                      LINK_PUBLIC
                      smtkCore
                      vtkSMTK
                      RemusClient)

target_include_directories(vtkCmbDiscreteModel
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SMTK_INCLUDE_DIRS}
    ${REMUS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${vtkCMBGeneral_SOURCE_DIR}
    ${vtkCMBFiltering_SOURCE_DIR}
    ${vtkCMBMeshing_SOURCE_DIR}
    ${vtkDiscreteModel_SOURCE_DIR}
    ${vtkDiscreteModel_SOURCE_DIR}/Model
  )

# build the ParaView plugin
add_subdirectory(Plugin)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif(BUILD_TESTING)
