add_subdirectory(cxx)

if(SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(python)
endif()

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(xml)
endif()
