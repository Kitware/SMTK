set(VTKLibSrcs
  vtkMeshMultiBlockSource.cxx
  vtkModelMultiBlockSource.cxx
  vtkModelRepresentation.cxx
  vtkModelSource.cxx
  vtkModelView.cxx
)

set(VTKLibHeaders
  vtkMeshMultiBlockSource.h
  vtkModelMultiBlockSource.h
  vtkModelRepresentation.h
  vtkModelSource.h
  vtkModelView.h
)

# Add compile definitions to force linking to implementation modules
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

# Don't do VTK header test. It is broken at the moment.
set(${vtk-module}_NO_HeaderTest 1)
set(VTK_INSTALL_NO_HEADERS 1)
vtk_module_library(${vtk-module} ${VTKLibSrcs})
target_include_directories(${vtk-module}
  PUBLIC
    $<BUILD_INTERFACE:${SMTK_SOURCE_DIR}>
    $<BUILD_INTERFACE:${SMTK_BINARY_DIR}>
    $<INSTALL_INTERFACE:include/smtk/${SMTK_VERSION}>)

target_link_libraries(${vtk-module}
  LINK_PUBLIC
    smtkCore
  LINK_PRIVATE
    ${Boost_LIBRARIES}
)

#has to be after the vtk_module_library
smtk_export_header(${vtk-module} Exports.h)

target_include_directories(${vtk-module}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Link *everything* in this directory to VTK modules
# This is required for the TestBuild targets.
# We must test whether each of the targets below exist
# because they are required when built into VTK but
# may not be present, especially since the OpenGL2
# backend became available.
set(SMTK_VTK_TARGET_LIST)
foreach(target
  vtkFiltersExtraction
  vtkFiltersGeometry
  vtkInteractionStyle
  vtkRenderingContext2D
  vtkRenderingFreeType
  vtkRenderingVolume
  vtkRenderingContextOpenGL2
  vtkRenderingOpenGL2
  vtkRenderingVolumeOpenGL2
  vtkRenderingGL2PSOpenGL2
  vtkIOGeometry
  vtkIOMPIParallel
  vtkIOMPIImage
  )
  if (TARGET ${target})
    list(APPEND SMTK_VTK_TARGET_LIST ${target})
  endif()
endforeach()

target_link_libraries(${vtk-module} PUBLIC ${SMTK_VTK_TARGET_LIST})

#install the headers
smtk_public_headers(${VTKLibHeaders})
# ... and make header compilation tests link properly:
smtk_install_library(${vtk-module})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  vtk_add_cs_wrapping(${vtk-module})
  smtk_install_library(${vtk-module}CS)
endif ()

# Potentially wrap in Python using VTK's wrapping methods, not shiboken.
# This makes the filters available for use with other VTK-wrapped filters.
if (SMTK_ENABLE_PYTHON_WRAPPING AND VTK_WRAP_PYTHON)
  vtk_wrap_python3(${vtk-module}Python SMTKVTKPY_SRCS "${VTKLibHeaders}")
  add_library(${vtk-module}PythonD ${SMTKVTKPY_SRCS} ${SMTKVTKPY_EXTRA_SRCS})
  python_add_module(${vtk-module}Python
    ${vtk-module}PythonInit.cxx
    ${vtk-module}PythonInitImpl.cxx
  )
  target_link_libraries(${vtk-module}PythonD
    PUBLIC
      ${vtk-module}
      vtkWrappingPythonCore
      vtkCommonExecutionModelPythonD
      vtkViewsInfovisPythonD
  )
  target_include_directories(${vtk-module}PythonD
    PRIVATE
    ${vtkPython_INCLUDE_DIRS}
    ${vtkWrappingPythonCore_INCLUDE_DIRS}
    )
  target_link_libraries(${vtk-module}Python
    PUBLIC
      vtkSMTKSourceExtPythonD
      ${PYTHON_LIBRARY}
      )
  target_include_directories(${vtk-module}Python
    PRIVATE
    ${vtkPython_INCLUDE_DIRS}
    ${vtkWrappingPythonCore_INCLUDE_DIRS}
    )
endif()

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_extension_vtk_source PUBLIC ${SMTK_VTK_TARGET_LIST})
  add_subdirectory(testing)
endif()
