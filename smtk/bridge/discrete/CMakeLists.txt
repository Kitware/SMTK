PROJECT(smtkDiscreteBridge)

# Only include the VTK use-file in subdirectories
# that require VTK include paths. Otherwise, command
# lines get too long on some platforms.
include("${VTK_USE_FILE}")
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/operation"
    "${CMAKE_CURRENT_SOURCE_DIR}/kernel"
    "${CMAKE_CURRENT_SOURCE_DIR}/kernel/Model"
    "${CMAKE_CURRENT_SOURCE_DIR}/kernel/Serialize"
    "${CMAKE_CURRENT_BINARY_DIR}/operation"
    "${CMAKE_CURRENT_BINARY_DIR}/kernel"
)

# Don't add the VTK or other subdirectories as they use the
# vtk_module_library() macro and vtk/module.cmake file.
# The pv_process_modules() macro finds all subdirectories
# containing a module.cmake file automatically.
include (vtkSMTKModules)
vtk_smtk_setup_module_environment("SMTKDiscrete")
vtk_smtk_process_modules()

# Build vtk moab reader before bridge
if(SMTK_BUILD_MOAB_READER)
  add_subdirectory(moabreader)
endif()

SET(DiscreteModelFiles
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
  operation/vtkMergeOperator.cxx
  operation/vtkMergeOperatorBase.cxx
  operation/vtkCMBModelReadOperator.cxx
  operation/vtkCreateModelEdgesOperator.cxx
  operation/vtkCreateModelEdgesOperatorBase.cxx
  operation/vtkSplitOperator.cxx
  operation/vtkSplitOperatorBase.cxx
  operation/vtkModelEntityOperatorBase.cxx
  operation/vtkDataSetRegionSurfaceFilter.cxx
  operation/vtkMasterPolyDataNormals.cxx
  operation/vtkMergeDuplicateCells.cxx
)

SET(DiscreteModelHeaders
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

  operation/vtkMergeOperator.h
  operation/vtkMergeOperatorBase.h
  operation/vtkCMBModelReadOperator.h
  operation/vtkCreateModelEdgesOperator.h
  operation/vtkCreateModelEdgesOperatorBase.h
  operation/vtkSplitOperator.h
  operation/vtkSplitOperatorBase.h
  operation/vtkModelEntityOperatorBase.h
  operation/vtkDataSetRegionSurfaceFilter.h
  operation/vtkMasterPolyDataNormals.h
  operation/vtkMergeDuplicateCells.h
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

set(discreteBridgeSrcs
  Bridge.cxx
  CreateEdgesOperator.cxx
  MergeOperator.cxx
  ReadOperator.cxx
  SplitFaceOperator.cxx
  ImportOperator.cxx
)

set(discreteBridgeHeaders
  Bridge.h
  CreateEdgesOperator.h
  MergeOperator.h
  ReadOperator.h
  SplitFaceOperator.h
  ImportOperator.h
)

# Normally this would be machine-generated from Bridge.json
# using the smtk_bridge_json CMake macro, but this version
# has been hand-edited to include preprocessor macros that
# add and remove engines and filetypes as support for them
# is known at compile time.
# include(SMTKBridgeJSON)
# smtk_bridge_json("${CMAKE_CURRENT_SOURCE_DIR}/Bridge.json" discreteBridgeJSON)

include(SMTKOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/CreateEdgesOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/MergeOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/ReadOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/SplitFaceOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/ImportOperator.sbt" unitOperatorXML)

add_library(smtkDiscreteBridge ${_module_src} ${discreteBridgeSrcs})

set (__private_dependencies)

if (VTK_WRAP_PYTHON)
  list(APPEND __private_dependencies
      vtkPythonInterpreter)
endif (VTK_WRAP_PYTHON)

#set smtkDiscreteBridge to publicly link to SMTKCore and privately to vtkSMTKDiscreteModel
target_link_libraries(smtkDiscreteBridge
  LINK_PUBLIC
    SMTKCore
  LINK_PRIVATE
    vtkRenderingCore
    vtkRenderingFreeType${VTK_RENDERING_BACKEND}
    vtkRendering${VTK_RENDERING_BACKEND}
   vtkSMTKDiscreteModel
   vtkFiltersCore
   vtkFiltersGeometry
   vtkIOParallel
   vtkRenderingCore
   vtkGeovisCore
   vtksys
   ${__private_dependencies}
  )

if(SMTK_BUILD_MOAB_READER)
  target_link_libraries(smtkDiscreteBridge
    LINK_PRIVATE
      vtkDiscreteMoabReader
  )
endif()


smtk_export_header(smtkDiscreteBridge discreteBridgeExports.h)

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(smtkDiscreteBridge PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# Install the headers.
smtk_public_headers(${discreteBridgeHeaders} ${DiscreteModelHeaders})

#install the library and exports the library when used from a build tree
smtk_install_library(smtkDiscreteBridge DEPENDS SMTKCore vtkSMTKDiscreteModel)

if(SMTK_BUILD_BRIDGE_PLUGIN AND SMTK_BUILD_DISCRETE_BRIDGE)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
  add_subdirectory(operation/testing)
endif()
