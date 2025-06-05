set(smtkResourcePythonTests
  testResource
  testResourceUnitSystem
)

# Additional tests that require SMTK_DATA_DIR
set(smtkResourcePythonDataTests
)

if ((SMTK_ENABLE_PARAVIEW_SUPPORT OR SMTK_ENABLE_VTK_SUPPORT) AND SMTK_ENABLE_MARKUP)
  if (TARGET VTK::FiltersParallelDIY2)
    if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
      list(APPEND smtkResourcePythonDataTests
        testCloneResources
      )
    endif()
  endif()
endif()

foreach (test ${smtkResourcePythonTests})
  smtk_add_test_python(${test}Py ${test}.py --src-dir=${smtk_SOURCE_DIR})
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkResourcePythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py --data-dir=${SMTK_DATA_DIR})
  endforeach()
endif()

add_executable(testPythonResource TestPythonResource.cxx)
target_compile_definitions(testPythonResource PRIVATE "SMTK_DATA_DIR=\"${SMTK_DATA_DIR}\"")
target_link_libraries(testPythonResource smtkCore)
add_test(NAME testPythonResource
  COMMAND $<TARGET_FILE:testPythonResource>
  "${CMAKE_CURRENT_SOURCE_DIR}/testResource.py")
set_tests_properties(testPythonResource PROPERTIES
  LABELS "Resource"
  ENVIRONMENT PYTHONDONTWRITEBYTECODE=1)
