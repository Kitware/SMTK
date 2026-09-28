set(unit_tests
  jobAgent.cxx
  jobDatabaseBindings.cxx
  # jobResourceInstance.cxx
)

if (SMTK_ENABLE_QT_SUPPORT)
  if (UNIX)
    # Do not run on Windows for now.
    # Windows CI does not currently provide the Bash interpreter required by
    # the ShellQueue portion of this test or Podman for ContainerQueue.
    list(APPEND unit_tests
      jobQueue.cxx
    )
  endif()
  list(APPEND extra_libraries
    smtkQtExt
  )
endif()

smtk_unit_tests(
  LABEL "Job"
  SOURCES ${unit_tests}
  LIBRARIES
    smtkCore
    SQLite::SQLite3
    ${extra_libraries}
)
