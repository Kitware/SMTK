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

set(smtkPVPluginSupportPythonTests
)

set(smtkPVPluginSupportPythonDataTests
)

if (SMTK_PARAVIEW_SUPPORT_ENABLED AND SMTK_ENABLE_POLYGON_SESSION)
  list (APPEND smtkPVPluginSupportPythonDataTests
    loadSMTKPluginInParaView)
endif()

foreach(test ${smtkPVPluginSupportPythonTests})
  if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py.in)
    file(GENERATE
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${test}.py
      INPUT ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py.in)
    smtk_add_test_python(${test}Py ${CMAKE_CURRENT_BINARY_DIR}/${test}.py)
  else()
    smtk_add_test_python(${test}Py ${testfile})
  endif()
  set_tests_properties( ${test}Py PROPERTIES LABELS "Plugin" )
endforeach()

if (SMTK_DATA_DIR)
  foreach(test ${smtkPVPluginSupportPythonDataTests})
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py.in)
      file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${test}.py
        INPUT ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py.in)
      smtk_add_test_python(${test}Py ${CMAKE_CURRENT_BINARY_DIR}/${test}.py
      --data-dir=${SMTK_DATA_DIR} )
    else()
      smtk_add_test_python(${test}Py ${test}.py
        --data-dir=${SMTK_DATA_DIR} )
    endif()
    set_tests_properties( ${test}Py PROPERTIES LABELS "Plugin" )
  endforeach()
endif()
