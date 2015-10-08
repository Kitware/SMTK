project(CMBModel)

find_package(SMTK REQUIRED)
find_package(Remus REQUIRED)

set(CMAKE_MODULE_PATH "${SMTK_ROOT_DIR}/lib/cmake/SMTK" ${CMAKE_MODULE_PATH})
include(SMTKOperatorXML)

SET(CmbModelFiles
  vtkCMBModelWriterV2.cxx
  vtkCMBModelWriterV4.cxx
  vtkCMBModelWriterV5.cxx
  vtkCMBParserBase.cxx
  vtkCMBParserV2.cxx
  vtkCMBParserV4.cxx
  vtkCMBParserV5.cxx
  vtkCMBModelOmicronMeshInputWriter.cxx
  vtkEnclosingModelEntityOperator.cxx
  vtkCMBModelActor.cxx
)

SET(CMBOperationFiles
  vtkADHExporterOperator.cxx
  vtkADHExporterOperatorBase.cxx
  vtkAppendSolids.cxx
  vtkMaterialOperator.cxx
  vtkMaterialOperatorBase.cxx
  vtkMergeOperator.cxx
  vtkMergeOperatorBase.cxx
  vtkCMBImportBCFileOperator.cxx
  vtkCMBIncorporateMeshOperator.cxx
  vtkCMBModelBuilder.cxx
  vtkCMBMapToCMBModel.cxx
  vtkCMBModelReader.cxx
  vtkCMBModelReadOperator.cxx
  vtkCMBModelWriterBase.cxx
  vtkCMBPolyDataProvider.cxx
  vtkCreateModelEdgesOperator.cxx
  vtkCreateModelEdgesOperatorBase.cxx
  vtkGenerateSimpleModelOperator.cxx
  vtkModelEntityGroupOperator.cxx
  vtkModelEntityGroupOperatorBase.cxx
  vtkModelEntityOperator.cxx
  vtkModelEntityOperatorBase.cxx
  vtkSeedGrowSelectionFilter.cxx
  vtkSelectionSplitOperator.cxx
  vtkSelectionSplitOperatorBase.cxx
  vtkSplitOperator.cxx
  vtkSplitOperatorBase.cxx
  vtkCMBModelStateOperatorBase.cxx
  vtkCMBModelStateOperator.cxx
  vtkCMBXMLBCSWriter.cxx
  vtkCMBModelOmicronMeshInputWriterBase.cxx
  vtkMergeDuplicateCells.cxx
  vtkMasterPolyDataNormals.cxx
  vtkCompleteShells.cxx
  vtkModelEdgeOperatorBase.cxx
  vtkModelEdgeOperator.cxx
  vtkEdgeSplitOperator.cxx
  vtkEdgeSplitOperatorBase.cxx
  vtkGeoTransformOperator.cxx
  vtkCMBModelPointsOperator.cxx
# This class should really be in ${Plugin_Src_Files},
# but it is linked from SimBuilderMeshPlugin
  vtkSMOperatorProxy.cxx
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

smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/CreateEdgesOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/MergeOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/ReadOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/SplitFaceOperator.sbt" unitOperatorXML)

# Disable manifests for plugins on Windows so that they do not need to resolve their dependencies.
if (MSVC)
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif(MSVC)

SET_SOURCE_FILES_PROPERTIES(
${CmbModelFiles}
WRAP_EXCLUDE
)

set(_module_src
  ${CmbModelFiles}
  ${CMBOperationFiles}
)
set(vtkCmbDiscreteModel_NO_HeaderTest 1)
set(VTK_INSTALL_NO_HEADERS 1)
vtk_module_library(vtkCmbDiscreteModel ${_module_src})

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
