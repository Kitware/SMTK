set(TEST_NAMES_WITH_BASELINES
  ImportGroupDelete
)

set(TESTS_WITH_BASELINES)

include(SMTKPVTestingMacros)

foreach(test ${TEST_NAMES_WITH_BASELINES})
  list(APPEND TESTS_WITH_BASELINES
    ${CMAKE_CURRENT_SOURCE_DIR}/${test}.xml
  )
  #TODO: this should be set for the entire project's xml tests, not individually
  set(${test}_USES_DIRECT_DATA ON)
endforeach()

if (SMTK_DATA_DIR)
smtk_add_client_tests(
  #LABEL "MarkupPlugin" # TODO: Label fixed by macro currently
  TEST_SCRIPTS ${TESTS_WITH_BASELINES}
  LOAD_PLUGINS
    smtkMarkupPlugin
    smtkResourcePlugin
    smtkOperationPlugin
    smtkGeometryPlugin
    smtkViewPlugin
    smtkPVServerExtPlugin
    smtkPQMarkupExtPlugin
    smtkPQComponentsPlugin
    smtkPQGuiComponentsPlugin
    smtkPQOperationsPanelPlugin
    smtkPQOperationViewsPlugin
    smtkPQReadersPlugin
    smtkPQWidgetsPlugin
  PLUGIN_PATHS
    $<TARGET_FILE_DIR:smtkMarkupPlugin>/..
    $<TARGET_FILE_DIR:smtkResourcePlugin>/..
    $<TARGET_FILE_DIR:smtkOperationPlugin>/..
    $<TARGET_FILE_DIR:smtkGeometryPlugin>/..
    $<TARGET_FILE_DIR:smtkViewPlugin>/..
    $<TARGET_FILE_DIR:smtkPQComponentsPlugin>/..
    $<TARGET_FILE_DIR:smtkPQGuiComponentsPlugin>/..
    $<TARGET_FILE_DIR:smtkPQOperationsPanelPlugin>/..
    $<TARGET_FILE_DIR:smtkPQOperationViewsPlugin>/..
    $<TARGET_FILE_DIR:smtkPQReadersPlugin>/..
    $<TARGET_FILE_DIR:smtkPVServerExtPlugin>/..
    $<TARGET_FILE_DIR:smtkPQMarkupExtPlugin>/..
    $<TARGET_FILE_DIR:smtkPQWidgetsPlugin>/..
  DATA_DIRECTORY ${PROJECT_SOURCE_DIR}/data
  BASELINE_DIR ${PROJECT_SOURCE_DIR}/data/baseline/smtk/widgets
  )
set(pv_tests)
foreach (test_file IN LISTS TESTS_WITH_BASELINES)
  get_filename_component(test_basename "${test_file}" NAME_WE)
  list(APPEND pv_tests "pv.${test_basename}")
endforeach ()
smtk_test_apply_sanitizer(${pv_tests})
endif()
