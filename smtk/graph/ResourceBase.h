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
  /** Erase all of the nodes from the \a node storage without updating the arcs.
   *  This is an internal method used for temporary removal, modification, and
   *  re-insertion in cases where \a node data that is indexed must be changed.
   *  In that case, arcs must not be modified.
   *
   * \return the number of nodes removed. Usually this is either 0 or 1, however the
   * implementation may define removal of > 1 nodes based on criteria other than id
   * or pointer address.
   */
  virtual std::size_t eraseNodes(const smtk::graph::ComponentPtr& node) = 0;

  /** Unconditionally insert the given \a node into the container.
   *  Does not check against NodeTypes to see whether the node type is
   *  allowed; this is assumed to have already been done.
   *
   * \return whether or not the insertion was successful.
   */
  virtual bool insertNode(const smtk::graph::ComponentPtr& node) = 0;

  ResourceBase(smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(manager)
  {
  }

  ResourceBase(const smtk::common::UUID& uid, smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(uid, manager)
  {
  }

  ResourceBase(ResourceBase&&) noexcept = default;

  friend Component;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_ResourceBase_h
