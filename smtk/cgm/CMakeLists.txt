include_directories(${CGM_INCLUDE_DIRS})
add_definitions(${CGM_DEFINES})

set(CGMLibSrcs
  Bridge.cxx
  CAUUID.cxx
  Engines.cxx
  ExportSolid.cxx
  ImportSolid.cxx
  Operator.cxx
  ReadOperator.cxx
  TDUUID.cxx
)

set(CGMLibHeaders
  Bridge.h
  CAUUID.h
  Engines.h
  ExportSolid.h
  ImportSolid.h
  Operator.h
  ReadOperator.h
  TDUUID.h
)

# Operators which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to cgmOperatorXML) since the operators themselves include
# the header in their implementations.
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/ReadOperator.sbt" cgmOperatorXML)

# Install the headers
smtk_public_headers(${CGMLibHeaders})
if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_cgm
    LINK_PRIVATE ${CGM_LIBRARIES}
  )
endif()

# Utility programs using CGM:
add_executable(cgm-convert
  cgm-convert.cxx
)
target_link_libraries(cgm-convert
  SMTKCore
  cgmSMTK
  ${CGM_LIBRARIES}
)

add_executable(facet-convert
  facet-convert.cxx
)
target_link_libraries(facet-convert
  SMTKCore
  ${CGM_LIBRARIES}
)

if (SMTK_BUILD_ParaView)
  include("${PARAVIEW_USE_FILE}")
  add_executable(vtk-to-cholla vtk-to-cholla.cxx)
  target_link_libraries(vtk-to-cholla
    SMTKCore
    vtkCommonDataModel
    vtkIOXML
    ${CGM_LIBRARIES}
  )
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
