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

set(unit_tests
  UnitTestCellTypes.cxx
  UnitTestCollection.cxx
  UnitTestManager.cxx
  UnitTestModelToMesh.cxx
  UnitTestQueryTypes.cxx
  UnitTestReadWriteHandles.cxx
  UnitTestTypeSet.cxx
)

set(unit_tests_which_require_data
  UnitTestAddFileToCollection.cxx
  UnitTestCellSet.cxx
  UnitTestCreateMesh.cxx
  UnitTestExtractTessellation.cxx
  UnitTestExportMesh2DM.cxx
  UnitTestExportMesh3DM.cxx
  UnitTestLoadMesh.cxx
  UnitTestMeshSet.cxx
  UnitTestPointConnectivity.cxx
  UnitTestPointSet.cxx
  UnitTestReadWriteMeshJSON.cxx
  UnitTestRemoveMeshes.cxx
  UnitTestTypeSetFromData.cxx
  UnitTestWriteMesh.cxx
)

smtk_unit_tests(
  LABEL "Mesh"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore smtkCoreModelTesting ${Boost_LIBRARIES}
)
