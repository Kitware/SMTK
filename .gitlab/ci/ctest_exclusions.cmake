set(test_exclusions
  pv.MeshSelection
)

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "fedora")
  list(APPEND test_exclusions
    # VTK lighting seems to be wrong.
    "^RenderMesh$"

    # Fails in CI; works locally. Needs investigation.
    "^pv\\.OpenExodusFile$"
    )
endif ()

string(REPLACE ";" "|" test_exclusions "${test_exclusions}")
if (test_exclusions)
  set(test_exclusions "(${test_exclusions})")
endif ()
