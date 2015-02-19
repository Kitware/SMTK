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

SET(_VTK_INSTALL_NO_DEVELOPMENT ${VTK_INSTALL_NO_DEVELOPMENT})
SET(VTK_INSTALL_NO_DEVELOPMENT ON)
# Don't do VTK header test. It is broken at the moment.
set(vtkSMTK_NO_HeaderTest 1)
vtk_module_library(vtkSMTK ${VTKLibSrcs})
SET(VTK_INSTALL_NO_DEVELOPMENT ${_VTK_INSTALL_NO_DEVELOPMENT})

target_link_libraries(vtkSMTK
  LINK_PUBLIC
    SMTKCore
)

#has to be after the vtk_module_library
smtk_export_header(vtkSMTK vtkSMTKExports.h)

target_include_directories(vtkSMTK
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(vtkSMTK PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()


# Link *everything* in this directory to VTK modules
# This is required for the TestBuild targets.
# We must test whether each of the targets below exist
# because they are required when built into VTK but
# may not be present, especially since the OpenGL2
# backend became available.
set(SMTK_VTK_TARGET_LIST)
foreach(target
        vtkInteractionStyle
        vtkRenderingContext2D
        vtkRenderingContext${VTK_RENDERING_BACKEND}
        vtkRendering${VTK_RENDERING_BACKEND}
        vtkRenderingMatplotlib
        vtkRenderingVolume${VTK_RENDERING_BACKEND}
        vtkRenderingFreeType${VTK_RENDERING_BACKEND}
        )
  if (TARGET ${target})
    list(APPEND SMTK_VTK_TARGET_LIST ${target})
  endif()
endforeach()

target_link_libraries(vtkSMTK PUBLIC ${SMTK_VTK_TARGET_LIST})

#install the headers
smtk_public_headers(${VTKLibHeaders})
# ... and make header compilation tests link properly:
smtk_install_library(vtkSMTK)

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_extension_vtk PUBLIC ${SMTK_VTK_TARGET_LIST})
  add_subdirectory(testing)
endif()
