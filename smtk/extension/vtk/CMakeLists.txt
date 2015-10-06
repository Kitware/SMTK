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
set(vtkSMTKExt_NO_HeaderTest 1)
vtk_module_library(vtkSMTKExt ${VTKLibSrcs})

target_link_libraries(vtkSMTKExt
  LINK_PUBLIC
    smtkCore
)

#has to be after the vtk_module_library
smtk_export_header(vtkSMTKExt Exports.h)

target_include_directories(vtkSMTKExt
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(vtkSMTKExt PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
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
  vtkRenderingMatplotlib
  vtkRenderingFreeType
  vtkRenderingVolume
  vtkRenderingContext${VTK_RENDERING_BACKEND}
  vtkRendering${VTK_RENDERING_BACKEND}
  vtkRenderingVolume${VTK_RENDERING_BACKEND}
  )
  if (TARGET ${target})
    list(APPEND SMTK_VTK_TARGET_LIST ${target})
  endif()
endforeach()

target_link_libraries(vtkSMTKExt PUBLIC ${SMTK_VTK_TARGET_LIST})

#install the headers
smtk_public_headers(${VTKLibHeaders})
# ... and make header compilation tests link properly:
smtk_install_library(vtkSMTKExt)

# Potentially wrap in Python using VTK's wrapping methods, not shiboken.
# This makes the filters available for use with other VTK-wrapped filters.
if (SMTK_ENABLE_PYTHON_WRAPPING AND VTK_WRAP_PYTHON)
  vtk_wrap_python3(vtkSMTKExtPython SMTKVTKPY_SRCS "${VTKLibHeaders}")
  add_library(vtkSMTKExtPythonD ${SMTKVTKPY_SRCS} ${SMTKVTKPY_EXTRA_SRCS})
  python_add_module(vtkSMTKExtPython
    vtkSMTKExtPythonInit.cxx
    vtkSMTKExtPythonInitImpl.cxx
  )
  target_link_libraries(vtkSMTKExtPythonD
    PUBLIC
      vtkSMTKExt
      vtkWrappingPythonCore
      vtkCommonExecutionModelPythonD
      vtkViewsInfovisPythonD
  )
  target_link_libraries(vtkSMTKExtPython
    PUBLIC
      vtkSMTKExtPythonD
      ${PYTHON_LIBRARY}
  )
endif()

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_extension_vtk PUBLIC ${SMTK_VTK_TARGET_LIST})
  add_subdirectory(testing)
endif()
