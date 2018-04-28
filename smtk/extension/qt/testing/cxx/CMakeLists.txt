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

if (SMTK_ENABLE_POLYGON_SESSION)
  set(unit_tests_headers
    qtPrintLog.h
  )
  set(unit_tests_headers_data
    unitQtComponentItem.h
  )

  set(unit_tests
    UnitTestEmittingStringBuffer.cxx
  )

  set(unit_tests_which_require_data
    unitQtComponentItem.cxx
  )

  set(unitQtComponentItem_EXTRA_ARGUMENTS "--exit")

  qt5_wrap_cpp(unit_tests_headers_moc ${unit_tests_headers})
  if (SMTK_DATA_DIR)
    qt5_wrap_cpp(unit_tests_headers_moc_data ${unit_tests_headers_data})
  endif ()

  smtk_unit_tests(LABEL "qt"
    SOURCES ${unit_tests}
    SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
    EXTRA_SOURCES ${unit_tests_headers_moc} ${unit_tests_headers_moc_data}
    LIBRARIES
      smtkQtExt
      smtkCore
      smtkCoreModelTesting
      smtkCoreResourceTesting
      smtkPolygonSession
      ${Boost_LIBRARIES}
  )
endif()
