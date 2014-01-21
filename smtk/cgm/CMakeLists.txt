include_directories(${CGM_INCLUDE_DIRS})
add_definitions(${CGM_DEFINES})

set(CGMLibSrcs
  CAUUID.cxx
  ExportSolid.cxx
  ImportSolid.cxx
  TDUUID.cxx
)

set(CGMLibHeaders
  CAUUID.h
  ExportSolid.h
  ImportSolid.h
  TDUUID.h
)

# Install the headers
smtk_public_headers(${CGMLibHeaders})
# ... and make header compilation tests link properly:
target_link_libraries(TestBuild_smtk_cgm
  LINK_PRIVATE ${CGM_LIBRARIES}
)

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

if (SMTK_BUILD_VTK)
  add_executable(vtk-to-cholla vtk-to-cholla.cxx)
  target_link_libraries(vtk-to-cholla
    SMTKCore
    ${VTK_MODULES}
    ${CGM_LIBRARIES}
  )
endif()

#if (SMTK_ENABLE_TESTING)
#  add_subdirectory(testing)
#endif()
