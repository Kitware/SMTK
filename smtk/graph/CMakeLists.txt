set(graphOperations
)

set(graphSrcs
  Component.cxx
  NodeSet.cxx
)

set(graphHeaders
  ArcImplementation.h
  ArcProperties.h
  ArcMap.h
  Component.h
  ExplicitArcs.h
  NodeProperties.h
  NodeSet.h
  Resource.h
  ResourceBase.h
  detail/TypeTraits.h
  evaluators/Dump.h
  filter/Grammar.h
  filter/TypeName.h
  json/ArcSerializer.h
  json/ArcDeserializer.h
  json/jsonResource.h
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
