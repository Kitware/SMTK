//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_OwnershipSemantics_h
#define smtk_graph_OwnershipSemantics_h

namespace smtk
{
namespace graph
{

/// Indicate whether one arc endpoint "owns" the other.
enum class OwnershipSemantics
{
  None,               //!< Neither endpoint owns its neighbor.
  FromNodeOwnsToNode, //!< The "from" node owns the "to" node.
  ToNodeOwnsFromNode  //!< The "to" node owns the "from" node.
  // NB: We may eventually add a "Programmatic" option for run-time
  //     or even per arc ownership semantics.
  // NB: Endpoints may not have mutual ownership.
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_OwnershipSemantics_h
