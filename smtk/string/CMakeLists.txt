set(stringClasses
  Manager
  Token
  json/DeserializationContext
  json/jsonManager
  json/jsonToken
)

set(stringSrcs)
set(stringHeaders)

foreach (class ${stringClasses})
  list(APPEND stringSrcs "${class}.cxx")
  list(APPEND stringHeaders "${class}.h")
endforeach()

#install the headers
smtk_public_headers(smtkCore ${stringHeaders})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
