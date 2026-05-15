set(unit_tests
  # jobResourceInstance.cxx
)

if (SMTK_ENABLE_QT_SUPPORT)
  list(APPEND unit_tests
    jobQueue.cxx
  )
  list(APPEND extra_libraries
    smtkQtExt
  )
endif()

smtk_unit_tests(
  LABEL "Job"
  SOURCES ${unit_tests}
  LIBRARIES
    smtkCore
    smtkQtExt # For ShellQueue
    ${extra_libraries}
)
