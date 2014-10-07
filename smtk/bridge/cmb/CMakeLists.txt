PROJECT(smtkCMBBridge)

# Only include the VTK use-file in subdirectories
# that require VTK include paths. Otherwise, command
# lines get too long on some platforms.
include("${VTK_USE_FILE}")
  list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")
  set(VTK_MAKE_INSTANTIATORS TRUE CACHE INTERNAL "" FORCE)

include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmbmodel"
    "${CMAKE_CURRENT_SOURCE_DIR}/discretemodel"
    "${CMAKE_CURRENT_SOURCE_DIR}/discretemodel/Model"
    "${CMAKE_CURRENT_SOURCE_DIR}/discretemodel/Serialize"
    "${CMAKE_CURRENT_BINARY_DIR}/cmbmodel"
    "${CMAKE_CURRENT_BINARY_DIR}/discretemodel"
)

# Don't add the VTK or other subdirectories as they use the
# vtk_module_library() macro and vtk/module.cmake file.
# The pv_process_modules() macro finds all subdirectories
# containing a module.cmake file automatically.
include (vtkSMTKModules)
vtk_smtk_setup_module_environment("SMTKCMB")
vtk_smtk_process_modules()

SET(CmbModelFiles
  cmbmodel/vtkCMBModelWriterV2.cxx
  cmbmodel/vtkCMBModelWriterV4.cxx
  cmbmodel/vtkCMBModelWriterV5.cxx
  cmbmodel/vtkCMBModelWriterBase.cxx
  cmbmodel/vtkCMBParserBase.cxx
  cmbmodel/vtkCMBParserV2.cxx
  cmbmodel/vtkCMBParserV4.cxx
  cmbmodel/vtkCMBParserV5.cxx
  cmbmodel/vtkCMBModelReader.cxx
)

SET(CMBOperationFiles
  cmbmodel/vtkMergeOperator.cxx
  cmbmodel/vtkMergeOperatorBase.cxx
  cmbmodel/vtkCMBModelReadOperator.cxx
  cmbmodel/vtkCreateModelEdgesOperator.cxx
  cmbmodel/vtkCreateModelEdgesOperatorBase.cxx
  cmbmodel/vtkSplitOperator.cxx
  cmbmodel/vtkSplitOperatorBase.cxx
)

SET(CmbModelHeaders
  cmbmodel/vtkCMBModelWriterV2.h
  cmbmodel/vtkCMBModelWriterV4.h
  cmbmodel/vtkCMBModelWriterV5.h
  cmbmodel/vtkCMBModelWriterBase.h
  cmbmodel/vtkCMBParserBase.h
  cmbmodel/vtkCMBParserV2.h
  cmbmodel/vtkCMBParserV4.h
  cmbmodel/vtkCMBParserV5.h
  cmbmodel/vtkCMBModelReader.h

  cmbmodel/vtkMergeOperator.h
  cmbmodel/vtkMergeOperatorBase.h
  cmbmodel/vtkCMBModelReadOperator.h
  cmbmodel/vtkCreateModelEdgesOperator.h
  cmbmodel/vtkCreateModelEdgesOperatorBase.h
  cmbmodel/vtkSplitOperator.h
  cmbmodel/vtkSplitOperatorBase.h
)


SET_SOURCE_FILES_PROPERTIES(
  cmbmodel/vtkCMBParserBase
  ABSTRACT
)

SET_SOURCE_FILES_PROPERTIES(
${CmbModelFiles}
WRAP_EXCLUDE
)

set(_module_src
  ${CmbModelFiles}
  ${CMBOperationFiles}
)

set(cmbBridgeSrcs
  Bridge.cxx
  CreateEdgesOperator.cxx
  MergeOperator.cxx
  ReadOperator.cxx
  SplitFaceOperator.cxx
)

set(cmbBridgeHeaders
  Bridge.h
  CreateEdgesOperator.h
  MergeOperator.h
  ReadOperator.h
  SplitFaceOperator.h
)

include(SMTKOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/CreateEdgesOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/MergeOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/ReadOperator.sbt" unitOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/SplitFaceOperator.sbt" unitOperatorXML)

add_library(smtkCMBBridge ${_module_src} ${cmbBridgeSrcs})

set (__private_dependencies)

if (VTK_WRAP_PYTHON)
  list(APPEND __private_dependencies
      vtkPythonInterpreter)
endif (VTK_WRAP_PYTHON)

#set smtkCMBBridge to publicly link to SMTKCore and privately to vtkSMTKDiscreteModel
target_link_libraries(smtkCMBBridge
  LINK_PUBLIC
    SMTKCore
  LINK_PRIVATE
   vtkSMTKDiscreteModel
   vtkFiltersCore
   vtkRenderingCore
   vtkGeovisCore
   vtksys
   ${__private_dependencies}
  )

smtk_export_header(smtkCMBBridge cmbBridgeExports.h)

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(smtkCMBBridge PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# Install the headers.
smtk_public_headers(${cmbBridgeHeaders} ${CmbModelHeaders})

#install the library and exports the library when used from a build tree
smtk_install_library(smtkCMBBridge DEPENDS SMTKCore vtkSMTKDiscreteModel)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
  add_subdirectory(cmbmodel/testing)
endif()
