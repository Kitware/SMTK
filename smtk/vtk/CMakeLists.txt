set(VTKLibSrcs
  vtkSMTKModelMultiBlockSource.cxx
  vtkSMTKModelRepresentation.cxx
  vtkSMTKModelSource.cxx
  vtkSMTKModelView.cxx
)

set(VTKLibHeaders
  vtkSMTKModelMultiBlockSource.h
  vtkSMTKModelRepresentation.h
  vtkSMTKModelSource.h
  vtkSMTKModelView.h
)

#install the headers
smtk_public_headers(${VTKLibHeaders})
# ... and make header compilation tests link properly:
target_link_libraries(TestBuild_smtk_vtk
  LINK_PRIVATE ${VTK_MODULES}
)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
