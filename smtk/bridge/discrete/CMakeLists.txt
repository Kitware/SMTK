PROJECT(smtkDiscreteSession)

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
vtk_smtk_setup_module_environment("SMTKDiscrete")
vtk_smtk_process_modules()

# Build vtk moab reader before session
if(SMTK_ENABLE_MOAB_SUPPORT)
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
  operation/vtkEdgeSplitOperator.cxx
  operation/vtkEdgeSplitOperatorBase.cxx
  operation/vtkMergeOperator.cxx
  operation/vtkMergeOperatorBase.cxx
  operation/vtkCMBModelReadOperator.cxx
  operation/vtkCreateModelEdgesOperator.cxx
  operation/vtkCreateModelEdgesOperatorBase.cxx
  operation/vtkSplitOperator.cxx
  operation/vtkSplitOperatorBase.cxx
  operation/vtkMasterPolyDataNormals.cxx
  operation/vtkMaterialOperator.cxx
  operation/vtkMaterialOperatorBase.cxx
  operation/vtkMergeDuplicateCells.cxx
  operation/vtkModelEntityGroupOperator.cxx
  operation/vtkModelEntityGroupOperatorBase.cxx
  operation/vtkModelEntityOperatorBase.cxx
  operation/vtkGenerateSimpleModelOperator.cxx
  operation/vtkSeedGrowSelectionFilter.cxx
  operation/vtkSelectionSplitOperator.cxx
  operation/vtkSelectionSplitOperatorBase.cxx
  operation/vtkCompleteShells.cxx
  operation/vtkCMBIncorporateMeshOperator.cxx
  operation/vtkEnclosingModelEntityOperator.cxx
)

SET(DiscreteModelHeaders
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

  operation/vtkEdgeSplitOperator.h
  operation/vtkEdgeSplitOperatorBase.h
  operation/vtkMergeOperator.h
  operation/vtkMergeOperatorBase.h
  operation/vtkCMBModelReadOperator.h
  operation/vtkCreateModelEdgesOperator.h
  operation/vtkCreateModelEdgesOperatorBase.h
  operation/vtkSplitOperator.h
  operation/vtkSplitOperatorBase.h
  operation/vtkMasterPolyDataNormals.h
  operation/vtkMaterialOperator.h
  operation/vtkMaterialOperatorBase.h
  operation/vtkMergeDuplicateCells.h
  operation/vtkModelEntityGroupOperator.h
  operation/vtkModelEntityGroupOperatorBase.h
  operation/vtkModelEntityOperatorBase.h
  operation/vtkGenerateSimpleModelOperator.h
  operation/vtkSeedGrowSelectionFilter.h
  operation/vtkSelectionSplitOperator.h
  operation/vtkSelectionSplitOperatorBase.h
  operation/vtkCompleteShells.h
  operation/vtkCMBIncorporateMeshOperator.h
  operation/vtkEnclosingModelEntityOperator.h
)


SET_SOURCE_FILES_PROPERTIES(
  operation/vtkCMBParserBase
  ABSTRACT
)

SET_SOURCE_FILES_PROPERTIES(
${DiscreteModelFiles}
WRAP_EXCLUDE
)

set(_module_src
  ${DiscreteModelFiles}
  ${DiscreteOperationFiles}
)

set(discreteSessionSrcs
  Session.cxx
  Operator.cxx
  ArrangementHelper.cxx
  BathymetryHelper.cxx
#  operators/CreateEdgesOperator.cxx
  operators/EntityGroupOperator.cxx
  operators/ImportOperator.cxx
  operators/MergeOperator.cxx
  operators/ReadOperator.cxx
  operators/SplitFaceOperator.cxx
  operators/GrowOperator.cxx
  operators/WriteOperator.cxx
  operators/RemoveModel.cxx
  operators/EdgeOperator.cxx
  operators/SetProperty.cxx
)

set(discreteSessionHeaders
  Session.h
  Operator.h
  ArrangementHelper.h
  BathymetryHelper.h
#  operators/CreateEdgesOperator.h
  operators/EntityGroupOperator.h
  operators/ImportOperator.h
  operators/MergeOperator.h
  operators/ReadOperator.h
  operators/SplitFaceOperator.h
  operators/GrowOperator.h
  operators/WriteOperator.h
  operators/RemoveModel.h
  operators/EdgeOperator.h
  operators/SetProperty.h
)

# Normally this would be machine-generated from Session.json
# using the smtk_session_json CMake macro, but this version
# has been hand-edited to include preprocessor macros that
# add and remove engines and filetypes as support for them
# is known at compile time.
# include(SMTKSessionJSON)
# smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" discreteSessionJSON)

include(SMTKOperatorXML)
# smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateEdgesOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/MergeOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ReadOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SplitFaceOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ImportOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EntityGroupOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/GrowOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/WriteOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/RemoveModel.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EdgeOperator.sbt" unitOperatorXML)
# NB: SetProperty operator reuses smtk::model::SetProperty's XML

add_library(smtkDiscreteSession ${_module_src} ${discreteSessionSrcs})

set (__private_dependencies)

if (VTK_WRAP_PYTHON)
  list(APPEND __private_dependencies vtkPythonInterpreter)
endif (VTK_WRAP_PYTHON)

if(SMTK_ENABLE_MOAB_SUPPORT)
  list(APPEND __private_dependencies vtkDiscreteMoabReader)
endif()

#set smtkDiscreteSession to publicly link to smtkCore and privately to vtkSMTKDiscreteModel
target_link_libraries(smtkDiscreteSession
  LINK_PUBLIC
    smtkCore
  LINK_PRIVATE
    vtkCommonExecutionModel
    vtkFiltersCore
    vtkFiltersGeometry
    vtkFiltersSources
    vtkGeovisCore
    vtkIOParallel
    vtkRenderingCore
    vtkRenderingFreeType
    vtkRendering${VTK_RENDERING_BACKEND}
    vtksys
    ${__private_dependencies}
    vtkSMTKDiscreteExt
    vtkSMTKDiscreteModel
  )


smtk_export_header(smtkDiscreteSession Exports.h)

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(smtkDiscreteSession PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# Install the headers.
smtk_public_headers(${discreteSessionHeaders} ${DiscreteModelHeaders})

#install the library and exports the library when used from a build tree
smtk_install_library(smtkDiscreteSession)

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)
  #extract the headers from discrete library we built to give them to shiboken

  sbk_wrap_library(smtkDiscreteSession
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${SMTK_SOURCE_DIR}/smtk
    LOCAL_INCLUDE_DIRECTORIES
      ${SMTK_SOURCE_DIR}/smtk/common
      ${SMTK_SOURCE_DIR}/smtk/attribute
      ${SMTK_SOURCE_DIR}/smtk/model
      ${SMTK_SOURCE_DIR}/smtk/session
      ${SMTK_SOURCE_DIR}/smtk/bridge/discrete
      ${SMTK_SOURCE_DIR}/smtk/simulation
      ${SMTK_SOURCE_DIR}/smtk/io
      ${SMTK_SOURCE_DIR}/smtk/view
      ${SMTK_SOURCE_DIR}/smtk
      ${SMTK_BINARY_DIR}/smtk
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/typesystem.xml
    HEADERS ${discreteSessionHeaders}
    DEPENDS
      smtkCore
  )
  target_include_directories(smtkDiscreteSessionPython
    PRIVATE
      "${CMAKE_CURRENT_BINARY_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}/operation"
      "${CMAKE_CURRENT_SOURCE_DIR}/kernel"
      "${CMAKE_CURRENT_SOURCE_DIR}/kernel/Model"
      "${CMAKE_CURRENT_SOURCE_DIR}/kernel/Serialize"
      "${CMAKE_CURRENT_BINARY_DIR}/operation"
      "${CMAKE_CURRENT_BINARY_DIR}/kernel"
  )
endif()

if(SMTK_ENABLE_PARAVIEW_SUPPORT AND SMTK_ENABLE_DISCRETE_SESSION)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
  add_subdirectory(operation/testing)
endif()
