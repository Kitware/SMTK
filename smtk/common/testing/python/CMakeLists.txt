set(smtkCommonPythonTests
  uuidGenerator
  datetimezonepairtest
  testURL
)

if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
  # Do not test python-based plugin loading on older ParaViews.
  list(APPEND smtkCommonPythonTests
    pluginLoader
  )
endif()

foreach (test ${smtkCommonPythonTests})
  smtk_add_test_python(${test}Py ${test}.py --src-dir=${smtk_SOURCE_DIR})
endforeach()
