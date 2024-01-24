set(graphOperations
  CreateArc
  CreateArcType
  Delete
  DeleteArc
)

set(graphSrcs
  ArcMap.cxx
  Component.cxx
  Directionality.cxx
  NodeSet.cxx
  Registrar.cxx
  evaluators/Dump.cxx
)

set(graphHeaders
  ArcImplementationBase.h
  ArcImplementation.h
  ArcProperties.h
  ArcMap.h
  ArcTraits.h
  Component.h
  Directionality.h
  ExplicitArcs.h
  Functions.h
  NodeProperties.h
  NodeSet.h
  OwnershipSemantics.h
  Registrar.h
  Resource.h
  ResourceBase.h
  RuntimeArc.h
  RuntimeArcEndpoint.h
  detail/TypeTraits.h
  evaluators/DeleteArcs.h
  evaluators/Dump.h
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

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
