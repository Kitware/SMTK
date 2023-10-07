set(graphOperations
  Delete
)

set(graphSrcs
  Component.cxx
  NodeSet.cxx
  evaluators/Dump.cxx
)

set(graphHeaders
  ArcImplementation.h
  ArcProperties.h
  ArcMap.h
  Component.h
  ExplicitArcs.h
  Functions.h
  NodeProperties.h
  NodeSet.h
  OwnershipSemantics.h
  Resource.h
  ResourceBase.h
  detail/TypeTraits.h
  evaluators/DeleteArcs.h
  evaluators/Dump.h
  evaluators/InsertUntypedArc.h
  evaluators/OwnersOf.h
  evaluators/OwnedBy.h
  filter/Grammar.h
  filter/TypeName.h
  json/ArcSerializer.h
  json/ArcDeserializer.h
  json/NodeSerializer.h
  json/NodeDeserializer.h
  json/jsonResource.h
)

foreach(operation ${graphOperations})
  list(APPEND graphSrcs operators/${operation}.cxx)
  list(APPEND graphHeaders operators/${operation}.h)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/operators/${operation}.sbt")
    smtk_encode_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/operators/${operation}.sbt"
      HEADER_OUTPUT headerName)
    list(APPEND graphHeaders "${headerName}")
    list(APPEND _graphDependencies ${headerName})
  endif()
endforeach()
add_custom_target(graphGenHeaders
  DEPENDS
    ${_graphDependencies}
)

# Install the headers:
smtk_public_headers(smtkCore ${graphHeaders})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
