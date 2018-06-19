if(SMTK_ENABLE_VTK_SUPPORT AND SMTK_ENABLE_PARAVIEW_SUPPORT)
  #if using paraview we need to explicitly look for vtkIOGDAL
  find_package(ParaView COMPONENTS vtkIOGDAL REQUIRED)
endif()

# Only include the VTK use-file in subdirectories
# that require VTK include paths. Otherwise, command
# lines get too long on some platforms.
include("${VTK_USE_FILE}")
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/operators"
    "${CMAKE_CURRENT_SOURCE_DIR}/operation"
    "${CMAKE_CURRENT_SOURCE_DIR}/kernel"
    "${CMAKE_CURRENT_SOURCE_DIR}/kernel/Model"
    "${CMAKE_CURRENT_SOURCE_DIR}/kernel/Serialize"
    "${CMAKE_CURRENT_BINARY_DIR}/operation"
    "${CMAKE_CURRENT_BINARY_DIR}/kernel"
    "${CMAKE_CURRENT_BINARY_DIR}/extension"
)

# Don't add the VTK or other subdirectories as they use the
# vtk_module_library() macro and vtk/module.cmake file.
# The pv_process_modules() macro finds all subdirectories
# containing a module.cmake file automatically.
include (smtkVTKModules)
vtk_smtk_setup_module_environment("SMTK")
vtk_smtk_process_modules()

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
  operators/CreateEdgesOperation.cxx
  operators/EntityGroupOperation.cxx
  operators/ImportOperation.cxx
  operators/LegacyReadResource.cxx
  operators/MergeOperation.cxx
  operators/ReadOperation.cxx
  operators/ReadResource.cxx
  operators/SplitFaceOperation.cxx
  operators/GrowOperation.cxx
  operators/WriteOperation.cxx
  operators/WriteResource.cxx
  operators/RemoveModel.cxx
  operators/EdgeOperation.cxx
  operators/SetProperty.cxx
)

set(discreteSessionHeaders
  Registrar.h
  Resource.h
  Session.h
  Operation.h
  ArrangementHelper.h
  operators/CreateEdgesOperation.h
  operators/EntityGroupOperation.h
  operators/ImportOperation.h
  operators/LegacyReadResource.h
  operators/MergeOperation.h
  operators/ReadOperation.h
  operators/ReadResource.h
  operators/SplitFaceOperation.h
  operators/GrowOperation.h
  operators/WriteOperation.h
  operators/WriteResource.h
  operators/RemoveModel.h
  operators/EdgeOperation.h
  operators/SetProperty.h
)

# Normally this would be machine-generated from Session.json
# using the smtk_session_json CMake macro, but this version
# has been hand-edited to include preprocessor macros that
# add and remove engines and filetypes as support for them
# is known at compile time.
# include(SMTKSessionJSON)
# smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" discreteSessionJSON)

include(SMTKOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateEdgesOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/MergeOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ReadOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/LegacyReadResource.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ReadResource.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SplitFaceOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ImportOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EntityGroupOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/GrowOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/WriteOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/WriteResource.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/RemoveModel.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EdgeOperation.sbt" unitOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SetProperty.sbt" unitOperationXML)

add_library(smtkDiscreteSession ${_module_src} ${discreteSessionSrcs})

set (__private_dependencies)

if (VTK_WRAP_PYTHON)
  list(APPEND __private_dependencies vtkPythonInterpreter)
endif (VTK_WRAP_PYTHON)

if(SMTK_ENABLE_MOAB_DISCRETE_READER)
  list(APPEND __private_dependencies vtkDiscreteMoabReader)
endif()

#set smtkDiscreteSession to publicly link to smtkCore and privately to vtkSMTKDiscreteModel
target_link_libraries(smtkDiscreteSession
  LINK_PUBLIC
    smtkCore
    smtkIOVTK
  LINK_PRIVATE
  ${Boost_LIBRARIES}
    vtkCommonExecutionModel
    vtkFiltersCore
    vtkFiltersGeometry
    vtkFiltersSources
    vtkGeovisCore
    vtkIOGDAL
    vtkIOParallel
    vtkRenderingCore
    vtkRenderingFreeType
    vtkRenderingGL2PSOpenGL2
    vtkRenderingContextOpenGL2
    vtkRenderingOpenGL2
    vtksys
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
smtk_public_headers(${discreteSessionHeaders} ${DiscreteModelHeaders})

#install the library and exports the library when used from a build tree
smtk_install_library(smtkDiscreteSession)

if(SMTK_ENABLE_PARAVIEW_SUPPORT AND SMTK_ENABLE_DISCRETE_SESSION)
  add_smtk_plugin(
    smtkDiscreteSessionPlugin "1.0"
    REGISTRAR smtk::bridge::discrete::Registrar
    MANAGERS smtk::operation::Manager smtk::resource::Manager
    LIBRARIES smtkCore smtkDiscreteSession)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_bridge_discrete LINK_PRIVATE vtkCommonCore)

  add_subdirectory(testing)
  add_subdirectory(operation/testing)
endif()
