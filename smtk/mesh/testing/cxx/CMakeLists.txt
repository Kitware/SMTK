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
  UnitTestQueryTypes.cxx
  UnitTestTypeSet.cxx
  )

set(unit_tests_which_require_data
  UnitTestAddFileToCollection.cxx
  UnitTestCellSet.cxx
  UnitTestLoadMesh.cxx
  UnitTestPointConnectivity.cxx
  UnitTestMeshSet.cxx
  UnitTestTypeSetFromData.cxx
  UnitTestWriteMesh.cxx
  )

smtk_unit_tests(LABEL "Mesh"
                SOURCES ${unit_tests}
                SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
                LIBRARIES SMTKCore ${Boost_LIBRARIES})
