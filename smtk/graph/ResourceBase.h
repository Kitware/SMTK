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

#include "smtk/PublicPointerDefs.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/TypeMap.h"

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
protected:
  struct SMTKCORE_EXPORT Compare
  {
    bool operator()(const std::shared_ptr<smtk::resource::Component>& lhs,
      const std::shared_ptr<smtk::resource::Component>& rhs) const;
  };

public:
  smtkTypeMacro(smtk::graph::ResourceBase);
  smtkSuperclassMacro(smtk::resource::DerivedFrom<ResourceBase, smtk::geometry::Resource>);

  typedef std::set<std::shared_ptr<smtk::resource::Component>, Compare> NodeSet;
  typedef smtk::common::TypeMap<smtk::common::UUID> ArcSet;

  std::shared_ptr<smtk::resource::Component> find(const smtk::common::UUID&) const override;

  void visit(std::function<void(const smtk::resource::ComponentPtr&)>& v) const override;

  const NodeSet& nodes() const { return m_nodes; }

  virtual const ArcSet& arcs() const = 0;
  virtual ArcSet& arcs() = 0;

protected:
  ResourceBase(smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(manager)
  {
  }

  ResourceBase(const smtk::common::UUID& uid, smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(uid, manager)
  {
  }

  NodeSet m_nodes;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_ResourceBase_h
