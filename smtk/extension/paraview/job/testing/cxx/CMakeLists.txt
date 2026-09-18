smtk_unit_tests(
  LABEL "Job"
  SOURCES TestArtifactStageTracker.cxx
  LIBRARIES smtkCore
)

add_executable(TestJobRunnerInitialization TestJobRunnerInitialization.cxx)
target_link_libraries(TestJobRunnerInitialization PRIVATE smtkPVJobExt)
add_test(NAME pv.jobRunnerInitialization COMMAND TestJobRunnerInitialization)
set_tests_properties(pv.jobRunnerInitialization PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
