smtk_unit_tests(
  LABEL "Job"
  SOURCES TestArtifactStageTracker.cxx
  LIBRARIES smtkCore
)

add_executable(TestJobRunnerInitialization TestJobRunnerInitialization.cxx)
target_link_libraries(TestJobRunnerInitialization PRIVATE smtkPVJobExt)
add_test(NAME pv.jobRunnerInitialization COMMAND TestJobRunnerInitialization)
set_tests_properties(pv.jobRunnerInitialization PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

if (NOT WIN32)
  add_executable(TestContainerEngineSettings TestContainerEngineSettings.cxx)
  target_link_libraries(TestContainerEngineSettings PRIVATE smtkPVJobExt smtkPVServerExt)
  add_dependencies(TestContainerEngineSettings smtkPVServerExtPlugin)
  add_test(NAME pv.containerEngineSettings
    COMMAND TestContainerEngineSettings $<TARGET_FILE:smtkPVServerExtPlugin> --disable-registry)
  set_tests_properties(pv.containerEngineSettings PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif ()
