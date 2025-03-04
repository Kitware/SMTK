//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/VertexUse.h"

#include "smtk/model/Chain.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Vertex.h"

namespace smtk
{
namespace model
{

Vertex VertexUse::vertex() const
{
  return this->cell().as<Vertex>();
}

smtk::model::Edges VertexUse::edges() const
{
  Edges result;
  EntityRefs all = this->bordantEntities(/*ofDimension = */ 1);
  for (EntityRefs::iterator it = all.begin(); it != all.end(); ++it)
  {
    if (it->isEdge())
      result.emplace_back(*it);
  }
  return result;
}

Chains VertexUse::chains() const
{
  Chains result;
  EntityRefArrangementOps::appendAllRelations(*this, HAS_SHELL, result);
  return result;
}

} // namespace model
} // namespace smtk
