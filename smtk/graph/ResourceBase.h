//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_ResourceBase_h
#define smtk_graph_ResourceBase_h

#include "smtk/geometry/Manager.h"
#include "smtk/geometry/Resource.h"

#include "smtk/graph/ArcMap.h"

#include <memory>
#include <string>
#include <tuple>
#include <typeindex>

namespace smtk
{
namespace graph
{

class Component;

/// A non-templated base class for graph resources. ResourceBase satisfies the
/// API of smtk::resource::Resource and isolates graph components from the traits
/// specification of smtk::graph::Resource.
class SMTKCORE_EXPORT ResourceBase
  : public smtk::resource::DerivedFrom<ResourceBase, smtk::geometry::Resource>
{
public:
  smtkTypeMacro(smtk::graph::ResourceBase);
  smtkSuperclassMacro(smtk::resource::DerivedFrom<ResourceBase, smtk::geometry::Resource>);

  virtual const ArcMap& arcs() const = 0;
  virtual ArcMap& arcs() = 0;

protected:
  /// Erase all nodes that component corresponds to according to the NodeStorage
  virtual bool eraseNode(const smtk::resource::ComponentPtr& component) = 0;
  /// Add a node without checking if the node is a valid Graph type
  virtual bool insertNode(const smtk::resource::ComponentPtr& component) = 0;

  ResourceBase(smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(manager)
  {
  }

  ResourceBase(const smtk::common::UUID& uid, smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(uid, manager)
  {
  }

  friend Component;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_ResourceBase_h
