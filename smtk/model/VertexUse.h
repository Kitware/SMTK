//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_VertexUse_h
#define __smtk_model_VertexUse_h

#include "smtk/model/UseEntity.h"

#include <vector>

namespace smtk {
  namespace model {

class Chain;
class Edge;
class Vertex;
typedef std::vector<Chain> Chains;
typedef std::vector<Edge> Edges;
typedef std::vector<EdgeUse> EdgeUses;

/**\brief A cursor subclass that provides methods specific to 0-d vertex cells.
  *
  */
class SMTKCORE_EXPORT VertexUse : public UseEntity
{
public:
  SMTK_CURSOR_CLASS(VertexUse,UseEntity,isVertexUse);

  Vertex vertex() const;
  Edges edges() const;
  Chains chains() const;
};

typedef std::vector<VertexUse> VertexUses;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_VertexUse_h
