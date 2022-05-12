//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_arcs_InsertUntypedArc_h
#define smtk_graph_arcs_InsertUntypedArc_h

#include "smtk/graph/ArcImplementation.h"
#include "smtk/graph/ArcProperties.h"

#include "smtk/common/TypeName.h"
#include "smtk/string/Token.h"

namespace smtk
{
namespace graph
{
namespace evaluators
{

/**\brief This is a functor that inserts an arc between two nodes whose
  *       type is unknown at compile time.
  *
  */
struct SMTKCORE_EXPORT InsertUntypedArc
{
  InsertUntypedArc() = default;

  static void begin(...) {}
  static void end(...) {}

  template<typename Impl, typename ArcTraits = typename Impl::Traits, typename Resource>
  void operator()(
    const Impl*,
    const Resource*,
    Component* fromNode,
    Component* toNode,
    smtk::string::Token arcType,
    bool& didInsert) const
  {
    if (arcType != smtk::common::typeName<ArcTraits>())
    {
      return;
    }
    auto typedFromNode = dynamic_cast<typename Impl::FromType*>(fromNode);
    auto typedToNode = dynamic_cast<typename Impl::ToType*>(toNode);
    if (typedFromNode && typedToNode)
    {
      didInsert |= typedFromNode->template outgoing<ArcTraits>().connect(typedToNode);
    }
  }
};

} // namespace evaluators
} // namespace graph
} // namespace smtk

#endif // smtk_graph_arcs_InsertUntypedArc_h
