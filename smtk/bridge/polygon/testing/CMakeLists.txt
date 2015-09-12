add_subdirectory(cxx)

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)
  add_subdirectory(python)
endif()
