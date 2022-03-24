//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_NodeSet_h
#define smtk_graph_NodeSet_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"

#include <functional>
#include <memory>
#include <set>

namespace smtk
{
namespace graph
{

class SMTKCORE_EXPORT NodeSet
{
  struct SMTKCORE_EXPORT Compare
  {
    bool operator()(
      const std::shared_ptr<smtk::resource::Component>& lhs,
      const std::shared_ptr<smtk::resource::Component>& rhs) const;
  };

  using NodeType = smtk::resource::ComponentPtr;
  using Container = std::set<NodeType, Compare>;

public:
  const Container& nodes() const;

  void visit(std::function<void(const smtk::resource::ComponentPtr&)>& v) const;
  NodeType find(const smtk::common::UUID& /* uuid */) const;
  smtk::resource::Component* component(const smtk::common::UUID& /* uuid */) const;

protected:
  std::size_t eraseNodes(const smtk::graph::ComponentPtr& node);
  bool insertNode(const smtk::graph::ComponentPtr& node);

private:
  Container m_nodes;
};

} // namespace graph
} // namespace smtk

#endif
