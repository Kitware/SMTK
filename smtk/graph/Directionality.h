//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_Directionality_h
#define smtk_graph_Directionality_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/string/Token.h"

namespace smtk
{
namespace graph
{

/**\brief An enumeration indicated whether an arc is directed or undirected.
  *
  */
enum Directionality : bool
{
  IsUndirected = false, //!< Arcs are not directional (a – b == b – a), also called bidirectional.
  IsDirected = true     //!< Arcs are directed (a → b != b → a), also called unidirectional.
};

/// Return a directionality given a string token.
Directionality SMTKCORE_EXPORT directionalityEnumerant(smtk::string::Token directionalityToken);

/// Return a string token for a Directionality enumerant.
smtk::string::Token SMTKCORE_EXPORT directionalityToken(Directionality directionalityEnumerant);

} // namespace graph
} // namespace smtk

#endif // smtk_graph_Directionality_h
