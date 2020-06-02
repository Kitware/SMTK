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

set(unit_tests)
set(unit_tests_which_require_data)
set(additional_sources)

set(external_libs ${Boost_LIBRARIES})

if (SMTK_ENABLE_VTK_SUPPORT)
  list(APPEND unit_tests_which_require_data
  RenderMesh.cxx)
  list(APPEND additional_sources
    smtkRegressionTestImage.cxx)
endif()

smtk_unit_tests(
  LABEL "vtk"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  EXTRA_SOURCES ${additional_sources}
  LIBRARIES
    smtkCore
    smtkMatplotlibExt
    ${external_libs}
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::InteractionStyle
    VTK::IOImage
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::TestingRendering
)

# This is required in order to get VTK's object factories hooked up.
if (TARGET UnitTests_smtk_extension_matplotlib_testing_cxx)
  vtk_module_autoinit(
    TARGETS UnitTests_smtk_extension_matplotlib_testing_cxx
    MODULES
      VTK::CommonCore
      VTK::CommonDataModel
      VTK::RenderingCore
      VTK::RenderingOpenGL2
      VTK::TestingRendering)
endif ()
