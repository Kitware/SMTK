set(VTKLibSrcs
  vtkModelMultiBlockSource.cxx
  vtkModelRepresentation.cxx
  vtkModelSource.cxx
  vtkModelView.cxx
)

set(VTKLibHeaders
  vtkModelMultiBlockSource.h
  vtkModelRepresentation.h
  vtkModelSource.h
  vtkModelView.h
)

# Add compile definitions to force linking to implementation modules
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

# Don't do VTK header test. It is broken at the moment.
set(vtkSMTK_NO_HeaderTest 1)

vtk_module_library(vtkSMTK ${VTKLibSrcs})
target_link_libraries(vtkSMTK
  LINK_PUBLIC
    SMTKCore
)
# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(vtkSMTK PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()


# Link *everything* in this directory to VTK modules
# This is required for the TestBuild targets.
link_libraries(
  ${VTK_MODULES}
  vtkInteractionStyle
  vtkRenderingContext2D
  vtkRenderingContextOpenGL
  vtkRenderingOpenGL
  vtkRenderingMatplotlib
  vtkRenderingVolumeOpenGL
  vtkRenderingFreeTypeOpenGL
)
#install the headers
smtk_public_headers(${VTKLibHeaders})
# ... and make header compilation tests link properly:
smtk_install_library(vtkSMTK)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
