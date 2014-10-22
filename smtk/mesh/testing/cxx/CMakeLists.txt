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
  UnitTestManager.cxx
  UnitTestCollection.cxx
  UnitTestQueryTypes.cxx
  )

set(unit_tests_which_require_data
  UnitTestLoadMesh.cxx
  )

smtk_unit_tests(LABEL "Mesh"
                SOURCES ${unit_tests}
                SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
                LIBRARIES SMTKCore)
