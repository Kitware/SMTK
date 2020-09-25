set(graphOperations
)

set(graphSrcs
  ResourceBase.cxx
  Component.cxx
)

set(graphHeaders
  Component.h
  Resource.h
  ResourceBase.h
  TypeTraits.h
  arcs/Arc.h
  arcs/Arcs.h
  arcs/OrderedArcs.h
  filter/Grammar.h
  filter/TypeName.h
)

foreach(operation ${graphOperations})
  smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operation}.sbt" defOpXML)
  list(APPEND graphSrcs operators/${operation}.cxx)
  list(APPEND graphHeaders operators/${operation}.h)
endforeach()

# Install the headers:
smtk_public_headers(smtkCore ${graphHeaders})

# if (SMTK_ENABLE_PYTHON_WRAPPING)
#   add_subdirectory(pybind11)
# endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
