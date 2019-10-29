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

set (unit_tests)

set (unit_tests_which_require_data
  UnitTestRead2dm.cxx
  UnitTestRead3dm.cxx
  UnitTestReadVtp.cxx
  UnitTestReadPts.cxx
  UnitTestReadXyz.cxx
  UnitTestReadObj.cxx
  UnitTestReadStl.cxx)

set (unit_tests_helpers
  smtkRegressionTestImage)

smtk_unit_tests(
  LABEL "vtk"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  EXTRA_SOURCES ${unit_tests_helpers}
  LIBRARIES
    smtkCore
    vtkSMTKReaderExt
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::TestingRendering
)

# This is required in order to get VTK's object factories hooked up.
vtk_module_autoinit(
  TARGETS UnitTests_smtk_extension_vtk_reader_testing_cxx
  MODULES
    vtkSMTKReaderExt
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::TestingRendering)
