if (ParaView_VERSION VERSION_LESS "5.10.0")
  # Do not run project tests with an old ParaView.
  # The set of plugins available is different.
  return()
endif()

set(TESTS_WITH_BASELINES
  ${CMAKE_CURRENT_SOURCE_DIR}/WorkletPanel.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/AgentPorts.xml
)

include(SMTKPVTestingMacros)

#TODO: this should be set for the entire project's xml tests
set(WorkletPanel_USES_DIRECT_DATA ON)
set(AgentPorts_USES_DIRECT_DATA ON)

if (SMTK_DATA_DIR)
smtk_add_client_tests(
  #LABEL "ProjectUI"
  TEST_SCRIPTS ${TESTS_WITH_BASELINES}
  LOAD_PLUGINS
    # Order is significant. Load non-GUI plugins first:
    smtkAttributePlugin
    smtkDefaultConfigurationPlugin
    smtkGeometryPlugin
    smtkModelPlugin
    smtkOperationPlugin
    smtkProjectPlugin
    smtkQtPlugin
    smtkResourcePlugin
    smtkTaskPlugin
    smtkViewPlugin
    smtkVTKOperationsPlugin

    # Load server-side plugins, esp. PVServerExt:
    smtkPVModelExtPlugin
    smtkPVServerExtPlugin

    # Load plugins that register GUI elements:
    smtkPQComponentsPlugin
    smtkPQCoreProjectPlugin
    smtkPQGroupingPlugin
    smtkPQGuiComponentsPlugin
    smtkPQGuiProjectPlugin
    smtkPQOperationsPanelPlugin
    smtkPQOperationViewsPlugin
    smtkPQReadersPlugin
    smtkPQWidgetsPlugin
  PLUGIN_PATHS
    $<TARGET_FILE_DIR:smtkAttributePlugin>/..
    $<TARGET_FILE_DIR:smtkDefaultConfigurationPlugin>/..
    $<TARGET_FILE_DIR:smtkGeometryPlugin>/..
    $<TARGET_FILE_DIR:smtkModelPlugin>/..
    $<TARGET_FILE_DIR:smtkOperationPlugin>/..
    $<TARGET_FILE_DIR:smtkPQComponentsPlugin>/..
    $<TARGET_FILE_DIR:smtkPQCoreProjectPlugin>/..
    $<TARGET_FILE_DIR:smtkPQGroupingPlugin>/..
    $<TARGET_FILE_DIR:smtkPQGuiComponentsPlugin>/..
    $<TARGET_FILE_DIR:smtkPQGuiProjectPlugin>/..
    $<TARGET_FILE_DIR:smtkPQOperationsPanelPlugin>/..
    $<TARGET_FILE_DIR:smtkPQOperationViewsPlugin>/..
    $<TARGET_FILE_DIR:smtkPQReadersPlugin>/..
    $<TARGET_FILE_DIR:smtkPQWidgetsPlugin>/..
    $<TARGET_FILE_DIR:smtkProjectPlugin>/..
    $<TARGET_FILE_DIR:smtkPVModelExtPlugin>/..
    $<TARGET_FILE_DIR:smtkPVServerExtPlugin>/..
    $<TARGET_FILE_DIR:smtkQtPlugin>/..
    $<TARGET_FILE_DIR:smtkResourcePlugin>/..
    $<TARGET_FILE_DIR:smtkTaskPlugin>/..
    $<TARGET_FILE_DIR:smtkViewPlugin>/..
    $<TARGET_FILE_DIR:smtkVTKOperationsPlugin>/..
  DATA_DIRECTORY ${PROJECT_SOURCE_DIR}/data
  BASELINE_DIR ${PROJECT_SOURCE_DIR}/data/baseline/smtk/project
  )
set(pv_tests)
foreach (test_file IN LISTS TESTS_WITH_BASELINES)
  get_filename_component(test_basename "${test_file}" NAME_WE)
  list(APPEND pv_tests "pv.${test_basename}")
endforeach ()
smtk_test_apply_sanitizer(${pv_tests})
endif()
