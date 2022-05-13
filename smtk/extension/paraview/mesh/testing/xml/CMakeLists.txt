set(TESTS_WITH_BASELINES
  ${CMAKE_CURRENT_SOURCE_DIR}/MeshSelection.xml
)

include(SMTKPVTestingMacros)

#TODO: this should be set for the entire project's xml tests
set(MeshSelection_USES_DIRECT_DATA ON)

if (SMTK_DATA_DIR)
smtk_add_client_tests(
  #LABEL "MeshSession"
  TEST_SCRIPTS ${TESTS_WITH_BASELINES}
  LOAD_PLUGINS
    smtkAttributePlugin
    smtkResourcePlugin
    smtkOperationPlugin
    smtkGeometryPlugin
    smtkViewPlugin
    smtkMeshPlugin
    smtkQtPlugin
    smtkPVServerExtPlugin
    smtkPQComponentsPlugin
    smtkPQGuiComponentsPlugin
    smtkPQReadersPlugin
    smtkPVMeshExtPlugin
  PLUGIN_PATHS
    $<TARGET_FILE_DIR:smtkAttributePlugin>/..
    $<TARGET_FILE_DIR:smtkResourcePlugin>/..
    $<TARGET_FILE_DIR:smtkOperationPlugin>/..
    $<TARGET_FILE_DIR:smtkGeometryPlugin>/..
    $<TARGET_FILE_DIR:smtkViewPlugin>/..
    $<TARGET_FILE_DIR:smtkMeshPlugin>/..
    $<TARGET_FILE_DIR:smtkQtPlugin>/..
    $<TARGET_FILE_DIR:smtkPVMeshExtPlugin>/..
    $<TARGET_FILE_DIR:smtkPQComponentsPlugin>/..
    $<TARGET_FILE_DIR:smtkPQGuiComponentsPlugin>/..
    $<TARGET_FILE_DIR:smtkPQReadersPlugin>/..
    $<TARGET_FILE_DIR:smtkPVServerExtPlugin>/..
  DATA_DIRECTORY ${PROJECT_SOURCE_DIR}/data
  BASELINE_DIR ${PROJECT_SOURCE_DIR}/data/baseline/smtk/mesh
  )
endif()
