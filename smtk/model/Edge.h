//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Edge_h
#define __smtk_model_Edge_h

#include "smtk/model/CellEntity.h"

//#include "smtk/common/Eigen.h" // For Vector3d

#include <vector>

namespace smtk {
  namespace model {

class EdgeUse;
class Vertex;
typedef std::vector<EdgeUse> EdgeUses;
typedef std::vector<Vertex> Vertices;

/**\brief A entityref subclass that provides methods specific to 1-d edge cells.
  *
  */
class SMTKCORE_EXPORT Edge : public CellEntity
{
public:
  SMTK_ENTITYREF_CLASS(Edge,CellEntity,isEdge);

  EdgeUses edgeUses() const;
  Vertices vertices() const;

  EdgeUse findOrAddEdgeUse(Orientation o, int sense = 0);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Edge_h
