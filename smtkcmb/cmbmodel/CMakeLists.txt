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

SET(CmbModelHeaders
  vtkCMBModelWriterV2.h
  vtkCMBModelWriterV4.h
  vtkCMBModelWriterV5.h
  vtkCMBModelWriterBase.h
  vtkCMBParserBase.h
  vtkCMBParserV2.h
  vtkCMBParserV4.h
  vtkCMBParserV5.h
  vtkCMBModelReader.h

  vtkMergeOperator.h
  vtkMergeOperatorBase.h
  vtkCMBModelReadOperator.h
  vtkCreateModelEdgesOperator.h
  vtkCreateModelEdgesOperatorBase.h
  vtkSplitOperator.h
  vtkSplitOperatorBase.h
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

# message(STATUS "CMAKE_CURRENT_SOURCE_DIR='${CMAKE_CURRENT_SOURCE_DIR}'")
# message(STATUS "CMAKE_CURRENT_BINARY_DIR='${CMAKE_CURRENT_BINARY_DIR}'")

set(vtkSMTKCMBModel_NO_HeaderTest 1)
vtk_module_library(vtkSMTKCMBModel ${_module_src})

#install the headers
smtk_public_headers(${CmbModelHeaders})
# ... and make header compilation tests link properly:
# smtk_install_library(vtkSMTKCMBModel)
  install(TARGETS vtkSMTKCMBModel
 #   EXPORT SMTK-targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

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
