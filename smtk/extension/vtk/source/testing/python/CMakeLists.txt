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

# Our only test here requires the markup resource.
if (SMTK_MARKUP_SUPPORT_ENABLED)
  set(smtkExtensionVTKSourcePythonDataTests
    iterateMultiBlockModel
  )

  if (SMTK_DATA_DIR)
    foreach(test ${smtkExtensionVTKSourcePythonDataTests})
      smtk_add_test_python(${test}Py ${test}.py
        --data-dir=${SMTK_DATA_DIR} )
      set_tests_properties( ${test}Py PROPERTIES LABELS "VTK" )
    endforeach()
  endif()
endif()
