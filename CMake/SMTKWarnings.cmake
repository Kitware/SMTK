if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # Lots of false positives with Boost's `variant` and `optional` types.
  string(APPEND CMAKE_CXX_FLAGS " -Wno-maybe-uninitialized")
endif ()
