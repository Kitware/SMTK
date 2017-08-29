add_subdirectory(cxx)

if(SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(python)
endif()
