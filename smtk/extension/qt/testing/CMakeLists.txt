#TODO: fix testing with local test classes on Windows
if (NOT WIN32)
  add_subdirectory(cxx)
endif ()
