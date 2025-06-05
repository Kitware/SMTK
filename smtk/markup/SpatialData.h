//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_SpatialData_h
#define smtk_markup_SpatialData_h

#include "smtk/markup/Component.h"

namespace smtk
{
namespace markup
{

class Domain;
class AssignedIds;

/**\brief Markup nodes that have spatial extents.
  *
  * Spatial data has a map from some abstract space (a Domain)
  * into some physical coordinate system that can be rendered
  * and analysed.
  *
  * Subclasses of SpatialData include DiscreteGeometry (where
  * the domain is represented as a complex of discrete primitive
  * shapes whose geometry is a (generally convex) combination of
  * corner points with a prescribed topology) and analytic shapes
  * (where the domain is a set of functions that map parameter
  * values into physical coordinates).
  */
class SMTKMARKUP_EXPORT SpatialData : public smtk::markup::Component
{
public:
  smtkTypeMacro(smtk::markup::SpatialData);
  smtkSuperclassMacro(smtk::markup::Component);

  template<typename... Args>
  SpatialData(Args&&... args)
    : smtk::markup::Component(std::forward<Args>(args)...)
  {
  }

  ~SpatialData() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  /// Return the set of domains in which this node participates.
  ///
  /// For discrete data (i.e., UnstructuredData, SubSet, SideSet, NodeSet), this
  /// usually includes an IdSpace names "cells" and another IdSpace named "points".
  /// (NodeSets only provide "points"; SideSets also include "sides").
  virtual std::unordered_set<Domain*> domains() const;

  /// Given a domain or its name, return an object recording the data's extent in the domain.
  ///
  /// For now, this method returns AssignedIds. In the future (when ParameterSpace
  /// domains are in use), it will return some yet-to-be-designed superclass of
  /// AssignedIds).
  ///
  /// Note that children only need to override the variant that accepts \a domainName
  /// as the default implementation of the other simply fetches the domain's name
  /// and passes it to the name-variant.
  virtual AssignedIds* domainExtent(Domain* domain) const;
  virtual AssignedIds* domainExtent(smtk::string::Token domainName) const;

  /// Return whether or not this node has its geometry blanked (i.e., not rendered).
  bool isBlanked() const;

  /// Set whether or not this node has its geometry blanked.
  ///
  /// Blanking a node's geometry is usually performed by an operation that wishes to
  /// keep the source geometry around but also permanently transform it in some reversible
  /// or editable way. While the markup session also allows transform properties
  /// attached to nodes to affect rendering, this can cause issues for downstream filters
  /// that need access to the transformed geometry (and do not wish to perform the transform
  /// themselves).
  ///
  /// This method returns true when the node's blanking state was modified.
  bool setBlanking(bool shouldBlank);

protected:
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_SpatialData_h
