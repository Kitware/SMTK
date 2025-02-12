set(unit_tests
  TestActiveTask.cxx
  TestConfigureOperation.cxx
  TestTaskBasics.cxx
  TestTaskJSON.cxx
  TestTaskPorts.cxx
  TestPortForwardingAgent.cxx
  # TestTaskUIState.cxx
)

find_package(Threads REQUIRED)

smtk_unit_tests(
  LABEL "Task"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)
