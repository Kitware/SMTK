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

# Add compile definitions to force linking to implementation modules
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

smtk_unit_tests(
  LABEL "vtk"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  EXTRA_SOURCES ${unit_tests_helpers}
  LIBRARIES
    smtkCore
    vtkRenderingGL2PSOpenGL2
    vtkSMTKSourceExt
    vtkSMTKReaderExt
    vtkTestingRendering
)
