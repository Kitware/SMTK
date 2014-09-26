project(SMTKCMBModel)

SET(CmbModelFiles
  vtkCMBModelWriterV2.cxx
  vtkCMBModelWriterV4.cxx
  vtkCMBModelWriterV5.cxx
  vtkCMBModelWriterBase.cxx
  vtkCMBParserBase.cxx
  vtkCMBParserV2.cxx
  vtkCMBParserV4.cxx
  vtkCMBParserV5.cxx
  vtkCMBModelReader.cxx
)

SET(CMBOperationFiles
  vtkMergeOperator.cxx
  vtkMergeOperatorBase.cxx
  vtkCMBModelReadOperator.cxx
  vtkCreateModelEdgesOperator.cxx
  vtkCreateModelEdgesOperatorBase.cxx
  vtkSplitOperator.cxx
  vtkSplitOperatorBase.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  vtkCMBParserBase
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
set(vtkSMTKCMBModel_NO_HeaderTest 1)
vtk_module_library(vtkSMTKCMBModel ${_module_src})

target_link_libraries(vtkSMTKCMBModel
                      LINK_PUBLIC
                      SMTKCore
                      vtkSMTKDiscreteModel
                      RemusClient)

target_include_directories(vtkSMTKCMBModel
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${vtkSMTKDiscreteModel_SOURCE_DIR}
    ${vtkSMTKDiscreteModel_SOURCE_DIR}/Model
  )

if(BUILD_TESTING)
  add_subdirectory(testing)
endif(BUILD_TESTING)
