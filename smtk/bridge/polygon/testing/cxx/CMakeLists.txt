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
  UnitTestPolygonCreateModel.cxx
  UnitTestPolygonCreateEdgeFromPoints.cxx)

set (unit_tests_which_require_data)

set(external_libs ${Boost_LIBRARIES})

smtk_unit_tests(
  LABEL "PolygonSession"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore smtkPolygonSession smtkCoreModelTesting ${external_libs}
)
