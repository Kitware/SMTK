//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/graph/Directionality.h"

namespace smtk
{
namespace graph
{

using namespace smtk::string::literals;

Directionality directionalityEnumerant(smtk::string::Token directionalityToken)
{
  switch (directionalityToken.id())
  {
    case "IsDirected"_hash:
    case "isdirected"_hash:
    case "directed"_hash:
    case "true"_hash:
      return Directionality::IsDirected;
    case "IsUndirected"_hash:
    case "isundirected"_hash:
    case "undirected"_hash:
    case "false"_hash:
    default:
      return Directionality::IsUndirected;
  }
}

smtk::string::Token directionalityToken(Directionality directionalityEnumerant)
{
  switch (directionalityEnumerant)
  {
    default:
    case Directionality::IsDirected:
      return "directed"_token;
      break;
    case Directionality::IsUndirected:
      return "undirected"_token;
      break;
  }
}

} // namespace graph
} // namespace smtk
