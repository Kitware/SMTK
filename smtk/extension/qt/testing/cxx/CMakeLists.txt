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

  set(unit_tests_headers
    qtPrintLog.h
    UnitTestEmittingStringBuffer.h
    )

  set(unit_tests_headers_data
    )

  set(unit_tests
    UnitTestEmittingStringBuffer.cxx
  )

  set(unit_tests_which_require_data
    )

  set(additional_libraries
    )

if (SMTK_ENABLE_POLYGON_SESSION)
  list(APPEND unit_tests_headers_data
    unitQtComponentItem.h
    )

  list(APPEND unit_tests_which_require_data
    unitQtComponentItem.cxx
    )

  list(APPEND additional_libraries
    smtkPolygonSession
    )

  set(unitQtComponentItem_EXTRA_ARGUMENTS "--exit")
endif ()

if (SMTK_ENABLE_UNSTABLE_TESTS)
  if (SMTK_ENABLE_MESH_SESSION)
    list(APPEND unit_tests_headers
      UnitTestOperationLauncher.h
      )
    list(APPEND unit_tests_which_require_data
      UnitTestOperationLauncher.cxx
      )

    list(APPEND additional_libraries
      smtkMeshSession
      )
  endif ()
endif ()

if (SMTK_DATA_DIR)
  list(APPEND unit_tests_headers
    ${unit_tests_headers_data})
endif ()

set(CMAKE_AUTOMOC 1)

smtk_unit_tests(LABEL "qt"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  EXTRA_SOURCES ${unit_tests_headers}
  LIBRARIES
  smtkQtExt
  smtkCore
  smtkCoreModelTesting
  smtkCoreResourceTesting
  ${additional_libraries}
  ${Boost_LIBRARIES}
  Threads::Threads
  MOAB
  )
