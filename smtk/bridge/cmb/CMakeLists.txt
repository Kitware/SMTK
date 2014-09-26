
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

add_library(cmbSMTK ${cmbBridgeSrcs})

# Install the headers
smtk_public_headers(${cmbBridgeHeaders})

#set cmbSMTK to publicly link to SMTKCore and privately to vtkCMBDiscreteModel
target_link_libraries(cmbSMTK
  LINK_PUBLIC
    SMTKCore
    vtkSMTK
  LINK_PRIVATE
    vtkCMBDiscreteModel
  )

target_include_directories(cmbSMTK
  PUBLIC
  ${CMBDiscrete_INCLUDE_DIRS}
  )

smtk_export_header(cmbSMTK cmbSMTKExports.h)

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(cmbSMTK PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

#install the library and exports the library when used from a build tree
smtk_install_library(cmbSMTK DEPENDS SMTKCore vtkSMTK)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
