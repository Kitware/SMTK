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

set (unit_tests
  TestCreateUniformGridOp.cxx
  TestMergeOp.cxx
  TestTransformOp.cxx
)

set (unit_tests_which_require_data
  UnitTestEulerRatio.cxx
  UnitTestImportFrom3dm.cxx
  UnitTestImportFromExodus.cxx
  UnitTestImportIntoResource.cxx
  UnitTestTopology.cxx
  UnitTestImportExport.cxx
)

set (unit_tests_serial_which_require_data
  TestMeshSessionReadWrite.cxx
)

set(external_libs ${Boost_LIBRARIES})

if (SMTK_ENABLE_VTK_SUPPORT)
  list (APPEND unit_tests_which_require_data UnitTestImportFromVTK.cxx)
  list (APPEND external_libs ${VTK_LIBRARIES} vtkSMTKSourceExt smtkIOVTK)
endif()

smtk_unit_tests(
  LABEL "MeshSession"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  SOURCES_SERIAL_REQUIRE_DATA ${unit_tests_serial_which_require_data}
  LIBRARIES smtkCore smtkMeshSession smtkCoreModelTesting ${external_libs}
)
