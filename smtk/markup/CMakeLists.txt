set(sources)
set(headers
  DomainFactory.h
  IdType.h
  Traits.h

  arcs/ReferencesToPrimaries.txx
  detail/DumpColors.h
  queries/SelectionFootprint.h
  testing/cxx/helpers.h
)
set(operations
  Create
  CreateArc
  CreateAnalyticShape
  CreateGroup
  Delete
  DumpGraph
  Import
  Read
  SetName
  TagIndividual
  Ungroup
  Write
)

list(APPEND classes
  ## Resource and data it holds
  Resource
  DomainMap

  ## Registration to managers
  Registrar

  ## Function domains
  Domain
  IdNature
  IdSpace
  AssignedIds
  IndirectAssignedIds
  SequentialAssignedIds

  ## Component subclasses
  Component
  BoundaryOperator
  ParameterSpace
  Index
  # Collection
  Ontology
  OntologyIdentifier
  # Frame
  Group
  AnalyticShape
  Plane
  Sphere
  Cone
  Box
  # ImplicitShape
  DiscreteGeometry
  ImageData
  # Grid
  # VolumeGrid
  # SurfaceGrid
  Subset
  # ExplicitSubset
  # RangedSubset
  # CompositeSubset
  NodeSet
  SideSet
  Field
  # LabelMap
  # Segmentation
  SpatialData
  Label
  # OntologyIdentifier
  Comment
  Feature
  # Landmark
  URL
  UnstructuredData

  # UI
  SubphraseGenerator

  arcs/ReferencesToPrimaries

  detail/NodeContainer

  json/jsonResource

  # Ontology import
  ontology/Source
  ontology/OwlRdfSource
)

if(MSVC)
  set_source_files_properties(
    json/jsonResource.cxx
    PROPERTIES
      COMPILE_FLAGS /bigobj)
endif()

# NB: Keep the next two foreach loops in order!
# Encode operation SBT files into headers and operations into classes.
foreach(operation ${operations})
  list(APPEND classes "operators/${operation}")
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/operators/${operation}.sbt")
    smtk_encode_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/operators/${operation}.sbt"
      TARGET_OUTPUT targetName
    )
    list(APPEND markupDependencies ${targetName})
  endif()
endforeach()

# Turn "classes" into source and headers entries
foreach(class ${classes})
  list(APPEND headers ${class}.h)
  list(APPEND sources ${class}.cxx)
endforeach()

add_library(smtkMarkup SHARED ${sources} ${headers})
smtk_export_header(smtkMarkup Exports.h)
add_dependencies(smtkMarkup ${markupDependencies})
target_link_libraries(smtkMarkup
  PUBLIC
    smtkCore
    Boost::boost
    VTK::CommonCore
    VTK::CommonDataModel
  PRIVATE
    VTK::IOImage
    VTK::IOXML
    VTK::IOGeometry
    VTK::FiltersCore
    VTK::FiltersExtraction
    VTK::FiltersGeometry
    VTK::FiltersParallelDIY2
    VTK::FiltersPoints
    Boost::filesystem
    VTK::hdf5
    smtkIOVTK
)
smtk_install_library(smtkMarkup)
smtk_public_headers(smtkMarkup ${headers})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

add_subdirectory(tools)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
