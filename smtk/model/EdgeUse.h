//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_EdgeUse_h
#define __smtk_model_EdgeUse_h

#include "smtk/model/UseEntity.h"

#include <vector>

namespace smtk {
  namespace model {

class Chain;
class Edge;
class EdgeUse;
class Face;
class Loop;
class Vertex;
class VertexUse;
typedef std::vector<Chain> Chains;
typedef std::vector<EdgeUse> EdgeUses;
typedef std::vector<Vertex> Vertices;
typedef std::vector<VertexUse> VertexUses;

/**\brief A cursor subclass that provides methods specific to 1-d edge cells.
  *
  */
class SMTKCORE_EXPORT EdgeUse : public UseEntity
{
public:
  SMTK_CURSOR_CLASS(EdgeUse,UseEntity,isEdgeUse);

  FaceUse faceUse() const;
  Loop loop() const;
  Edge edge() const;
  Chains chains() const;
  VertexUses vertexUses() const;
  Vertices vertices() const;

  EdgeUse ccwUse() const;
  EdgeUse cwUse() const;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_EdgeUse_h
