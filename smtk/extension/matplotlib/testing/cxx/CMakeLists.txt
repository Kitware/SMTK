#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================

set(additional_sources)

if (SMTK_ENABLE_VTK_SUPPORT)
  list(APPEND additional_sources
    ${PROJECT_SOURCE_DIR}/smtk/extension/vtk/reader/testing/cxx/smtkRegressionTestImage.cxx
    )
endif()

add_executable(RenderMesh RenderMesh.cxx ${additional_sources})

# Include pybind and python directories
target_include_directories(RenderMesh PRIVATE ${PYBIND11_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS})

# Add hooks to the data and scratch directories
target_compile_definitions(RenderMesh PRIVATE "SMTK_DATA_DIR=\"${SMTK_DATA_DIR}\"")
target_compile_definitions(RenderMesh PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")

set(additional_libraries
  ${PYTHON_LIBRARIES}
  ${Boost_LIBRARIES}
  )

# We use VTK to perform an image comparison. If it is not present, the check is skipped
if (SMTK_ENABLE_VTK_SUPPORT)

  # Add compile definitions to force linking to implementation modules
  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

# Do not enable VTK integration for testing
#  target_compile_definitions(RenderMesh PRIVATE "USE_VTK=1")

  list(APPEND additional_libraries
    vtkRenderingGL2PSOpenGL2
    vtkSMTKSourceExt
    vtkSMTKReaderExt
    vtkIOImage
    vtkInteractionImage
    vtkTestingRendering
    )
endif()

target_link_libraries(RenderMesh smtkCore smtkCoreModelTesting ${additional_libraries})

if (SMTK_DATA_DIR)
  add_test(NAME RenderMesh COMMAND $<TARGET_FILE:RenderMesh>)
  set_tests_properties(RenderMesh PROPERTIES
                       LABELS "ExtMatplotlib")
endif()
