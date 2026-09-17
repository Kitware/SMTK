add_executable(TestTaskPostprocessing TestTaskPostprocessing.cxx)
target_link_libraries(TestTaskPostprocessing PRIVATE smtkPQProjectExt)
add_test(NAME pv.taskPostprocessing COMMAND TestTaskPostprocessing)
set_tests_properties(pv.taskPostprocessing PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Set SMTK_TEST_READER_PLUGIN to an external CorpsFoam plugin path when running
# this test to also exercise real job-result files and output-port routing.
