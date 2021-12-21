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

#include "smtk/resource/Component.h"

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

protected:
  bool eraseNode(const smtk::resource::ComponentPtr& component);
  bool insertNode(const smtk::resource::ComponentPtr& component);

private:
  Container m_nodes;
};

} // namespace graph
} // namespace smtk

#endif
