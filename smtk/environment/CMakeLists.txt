set(environmentSrcs
  Environment.cxx
)

set(environmentHeaders
  Environment.h
)

add_library(smtkEnvironment ${environmentSrcs})
target_link_libraries(smtkEnvironment PUBLIC smtkCore)
smtk_export_header(smtkEnvironment Exports.h)
smtk_public_headers(${environmentHeaders})
smtk_install_library(smtkEnvironment)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
