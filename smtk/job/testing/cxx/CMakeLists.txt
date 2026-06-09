set(unit_tests
  # jobResourceInstance.cxx
)

if (SMTK_ENABLE_QT_SUPPORT)
  if (UNIX)
    # Do not run on Windows for now.
    # The problem is that we don't have CI set up with podman+wsl2
    # and we don't support ShellQueue on Windows at all.
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
    ${extra_libraries}
)
