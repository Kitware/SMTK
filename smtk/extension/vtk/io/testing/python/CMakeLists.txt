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

set(smtkExtensionVTKIOPythonDataTests
  MultiScaleConverter
)

if (SMTK_USE_PYBIND11)
  set(smtkExtensionVTKIOPythonDataTests
    ${smtkExtensionVTKIOPythonDataTests}
    unitImportExportVTKData
  )
endif()

if (SMTK_DATA_DIR
    AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  foreach(test ${smtkExtensionVTKIOPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR} )
    set_tests_properties( ${test}Py PROPERTIES LABELS "IO" )
  endforeach()
endif()
