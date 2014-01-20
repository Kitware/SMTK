include_directories(${CGM_INCLUDE_DIRS})
add_definitions(${CGM_DEFINES})

add_executable(cgm-convert
  cgm-convert.cxx
)
target_link_libraries(cgm-convert
  SMTKCore
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
