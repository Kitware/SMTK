set(TESTS_WITH_BASELINES
  ${CMAKE_CURRENT_SOURCE_DIR}/BoxWidget.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/ConeWidget.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/FrameWidget.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/LineWidget.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/PlaneWidget.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/PointWidget.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/SphereWidget.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/SplineWidget.xml
)

include(SMTKPVTestingMacros)

#TODO: this should be set for the entire project's xml tests
set(BoxWidget_USES_DIRECT_DATA ON)
set(ConeWidget_USES_DIRECT_DATA ON)
set(FrameWidget_USES_DIRECT_DATA ON)
set(LineWidget_USES_DIRECT_DATA ON)
set(PlaneWidget_USES_DIRECT_DATA ON)
set(PointWidget_USES_DIRECT_DATA ON)
set(SphereWidget_USES_DIRECT_DATA ON)
set(SplineWidget_USES_DIRECT_DATA ON)

if (SMTK_DATA_DIR)
smtk_add_client_tests(
  #LABEL "MeshSession"
  TEST_SCRIPTS ${TESTS_WITH_BASELINES}
  LOAD_PLUGINS
    smtkResourcePlugin
    smtkOperationPlugin
    smtkGeometryPlugin
    smtkViewPlugin
    smtkPVServerExtPlugin
    smtkPQComponentsPlugin
    smtkPQGuiComponentsPlugin
    smtkPQReadersPlugin
    smtkPQWidgetsPlugin
  PLUGIN_PATHS
    $<TARGET_FILE_DIR:smtkResourcePlugin>/..
    $<TARGET_FILE_DIR:smtkOperationPlugin>/..
    $<TARGET_FILE_DIR:smtkGeometryPlugin>/..
    $<TARGET_FILE_DIR:smtkViewPlugin>/..
    $<TARGET_FILE_DIR:smtkPQComponentsPlugin>/..
    $<TARGET_FILE_DIR:smtkPQGuiComponentsPlugin>/..
    $<TARGET_FILE_DIR:smtkPQReadersPlugin>/..
    $<TARGET_FILE_DIR:smtkPVServerExtPlugin>/..
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
