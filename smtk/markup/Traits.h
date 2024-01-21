//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Traits_h
#define smtk_markup_Traits_h
/*!\file */

#include "smtk/markup/Exports.h"

#include "smtk/graph/OwnershipSemantics.h"
#include "smtk/markup/detail/NodeContainer.h"

#include "smtk/markup/arcs/ReferencesToPrimaries.h"

#include <tuple>

namespace smtk
{
namespace markup
{

// Forward declare node types
class AnalyticShape;
class AssignedIds;
class Box;
class Collection;
class Comment;
class Component;
class CompositeSubset;
class Cone;
class DiscreteGeometry;
class ExplicitSubset;
class Feature;
class Field;
class Frame;
class Grid;
class Group;
class ImageData;
class ImplicitShape;
class Label;
class LabelMap;
class Landmark;
class NodeSet;
class Ontology;
class OntologyIdentifier;
class Plane;
class RangedSubset;
class References;
class Segmentation;
class SideSet;
class SpatialData;
class Sphere;
class Subset;
class SurfaceGrid;
class URL;
class UnstructuredData;
class VolumeGrid;

namespace arcs
{
/// Arcs connecting groups to their members and vice-versa.
struct SMTKMARKUP_EXPORT GroupsToMembers
{
  using FromType = Group;
  using ToType = Component;
  using Directed = std::true_type;
  static constexpr graph::OwnershipSemantics semantics =
    graph::OwnershipSemantics::ToNodeOwnsFromNode;
};

/// Arcs connecting labels to their subjects and vice-versa.
struct SMTKMARKUP_EXPORT LabelsToSubjects
{
  using FromType = Label;
  using ToType = Component;
  using Directed = std::true_type;
};

/// Arcs connecting URLs to data held at its location.
struct SMTKMARKUP_EXPORT URLsToData
{
  using FromType = URL;
  using ToType = Component;
  using Directed = std::true_type;
  static constexpr std::size_t maxInDegree = 1; // Data has a single source.
};

struct SMTKMARKUP_EXPORT URLsToImportedData
{
  using FromType = URL;
  using ToType = Component;
  using Directed = std::true_type;
};

/// Arcs connecting ontology identifiers to their subjects.
struct SMTKMARKUP_EXPORT OntologyIdentifiersToIndividuals
{
  using FromType = OntologyIdentifier;
  using ToType = Component;
  using Directed = std::true_type;
  /// An ontology identifier cannot be deleted if it has any named individuals attached.
  static constexpr graph::OwnershipSemantics semantics =
    graph::OwnershipSemantics::ToNodeOwnsFromNode;
};

/// Arcs connecting an ontology identifier to other identifiers derivde from it.
struct SMTKMARKUP_EXPORT OntologyIdentifiersToSubtypes
{
  using FromType = OntologyIdentifier;
  using ToType = OntologyIdentifier;
  using Directed = std::true_type;
};

/// Arcs connecting an ontology to all of its identifiers.
struct SMTKMARKUP_EXPORT OntologyToIdentifiers
{
  using FromType = Ontology;
  using ToType = OntologyIdentifier;
  using Directed = std::true_type;
  static constexpr std::size_t maxInDegree = 1; // Identifiers have a single parent ontology.
};

/// Arcs connecting spatial data (defining a shape) to functions defined over those spaces.
struct SMTKMARKUP_EXPORT FieldsToShapes
{
  using FromType = Field;
  using ToType = SpatialData;
  using Directed = std::true_type;
  static constexpr std::size_t minInDegree = 1; // Fields must have a domain.
  static constexpr std::size_t maxInDegree = 1; // Fields have a single domain (for now).
};

/// Arcs connecting boundary shapes to the higher-dimensional shapes they bound.
/// The "from" node must always be the lower-dimensional object.
struct SMTKMARKUP_EXPORT BoundariesToShapes
{
  using FromType = SpatialData;
  using ToType = SpatialData;
  using Directed = std::true_type;
  // You cannot delete a boundary without also deleting the things it bounds:
  static constexpr graph::OwnershipSemantics semantics =
    graph::OwnershipSemantics::ToNodeOwnsFromNode;
};
} // namespace arcs

// Forward-declare domain types
class BoundaryOperator;
class IdSpace;
class Index;
class ParameterSpace;

/**\brief Traits that describe markup node and arc types.
  *
  */
struct SMTKMARKUP_EXPORT Traits
{
  using NodeTypes = std::tuple<
    AnalyticShape,
    // AssignedIds,
    Box,
    // Collection,
    Comment,
    Component,
    // CompositeSubset,
    Cone,
    DiscreteGeometry,
    // ExplicitSubset,
    Feature,
    Field,
    // Frame,
    // Grid,
    Group,
    ImageData,
    // ImplicitShape,
    Label,
    // LabelMap,
    Landmark,
    NodeSet,
    Ontology,
    OntologyIdentifier,
    Plane,
    // RangedSubset,
    // References,
    // Segmentation,
    SideSet,
    SpatialData,
    Sphere,
    Subset,
    // SurfaceGrid,
    URL,
    UnstructuredData
    // VolumeGrid
    >;
  using ArcTypes = std::tuple<
    arcs::BoundariesToShapes,
    arcs::FieldsToShapes,
    arcs::GroupsToMembers,
    arcs::LabelsToSubjects,
    arcs::OntologyIdentifiersToIndividuals,
    arcs::OntologyIdentifiersToSubtypes,
    arcs::OntologyToIdentifiers,
    arcs::ReferencesToPrimaries,
    arcs::URLsToData,
    arcs::URLsToImportedData>;
  using NodeContainer = detail::NodeContainer;
  using DomainTypes = std::tuple<BoundaryOperator, IdSpace, Index, ParameterSpace>;
};
} // namespace markup
} // namespace smtk

// Include node types

#include "smtk/markup/AnalyticShape.h"
#include "smtk/markup/AssignedIds.h"
#include "smtk/markup/Box.h"
// #include "smtk/markup/Collection.h"
#include "smtk/markup/Comment.h"
// #include "smtk/markup/Component.h"
// #include "smtk/markup/CompositeSubset.h"
#include "smtk/markup/Cone.h"
#include "smtk/markup/DiscreteGeometry.h"
// #include "smtk/markup/ExplicitSubset.h"
#include "smtk/markup/Feature.h"
#include "smtk/markup/Field.h"
// #include "smtk/markup/Frame.h"
// #include "smtk/markup/Grid.h"
#include "smtk/markup/Group.h"
#include "smtk/markup/ImageData.h"
// #include "smtk/markup/ImplicitShape.h"
#include "smtk/markup/Label.h"
// #include "smtk/markup/LabelMap.h"
#include "smtk/markup/Landmark.h"
#include "smtk/markup/NodeSet.h"
#include "smtk/markup/Ontology.h"
#include "smtk/markup/OntologyIdentifier.h"
#include "smtk/markup/Plane.h"
// #include "smtk/markup/RangedSubset.h"
// #include "smtk/markup/References.h"
// #include "smtk/markup/Segmentation.h"
#include "smtk/markup/SideSet.h"
#include "smtk/markup/SpatialData.h"
#include "smtk/markup/Sphere.h"
#include "smtk/markup/Subset.h"
// #include "smtk/markup/SurfaceGrid.h"
#include "smtk/markup/URL.h"
#include "smtk/markup/UnstructuredData.h"
// #include "smtk/markup/VolumeGrid.h"

// Include domain types
#include "smtk/markup/BoundaryOperator.h"
#include "smtk/markup/Domain.h"
#include "smtk/markup/IdSpace.h"
#include "smtk/markup/Index.h"
#include "smtk/markup/ParameterSpace.h"

#endif
