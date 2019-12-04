set(test_exclusions
  # Issue #296.
  "elevateMeshOnStructuredGridPy"
  "pv.OpenExodusFile"
  "TestReadWrite"
)
string(REPLACE ";" "|" test_exclusions "${test_exclusions}")
if (test_exclusions)
  set(test_exclusions "(${test_exclusions})")
endif ()
