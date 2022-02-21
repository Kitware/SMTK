set(test_exclusions)

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows")
  list(APPEND test_exclusions
    # PATH setup is needed for these to work. See:
    # https://gitlab.kitware.com/paraview/paraview/-/merge_requests/5036 and
    # https://gitlab.kitware.com/cmake/cmake/-/merge_requests/6299
    "^pv\\."

    # segfault; needs investigation (#449).
    "^ImportMultipleFiles$"
    )
endif ()

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "fedora")
  list(APPEND test_exclusions
    # VTK lighting seems to be wrong.
    "^RenderMesh$"
    # QTest::qWaitForWindowActive fails on CI machines but works locally
    "^UnitTestDoubleClickButton$"
    )
endif ()

string(REPLACE ";" "|" test_exclusions "${test_exclusions}")
if (test_exclusions)
  set(test_exclusions "(${test_exclusions})")
endif ()
