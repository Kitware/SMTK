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

set (unit_tests_which_require_data)

if (ENABLE_PYARC_BINDINGS)
  list (APPEND unit_tests_which_require_data
    ExportToPyARCOp.cxx)
endif()

smtk_unit_tests(
  LABEL "RGGSession"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore smtkRGGSession ${Boost_LIBRARIES})

# All tests for this session require PyARC imported as a module from the
# embedded python interpreter. If PyARC is built using the superbuild, then the
# test environment will have access to it.
foreach (test ${unit_tests})
  get_filename_component(tname ${test} NAME_WE)
  set_tests_properties(${tname} PROPERTIES ENVIRONMENT "${smtk_python_test_environment}")
endforeach ()
foreach (test ${unit_tests_which_require_data})
  get_filename_component(tname ${test} NAME_WE)
  set_tests_properties(${tname} PROPERTIES ENVIRONMENT "${smtk_python_test_environment}")
endforeach ()
