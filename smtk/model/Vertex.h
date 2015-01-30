//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Vertex_h
#define __smtk_model_Vertex_h

#include "smtk/model/CellEntity.h"

//#include "smtk/common/Eigen.h" // For Vector3d

#include <vector>

namespace smtk {
  namespace model {

class Edge;
typedef std::vector<Edge> Edges;

/**\brief A entityref subclass that provides methods specific to 0-d vertex cells.
  *
  */
class SMTKCORE_EXPORT Vertex : public CellEntity
{
public:
  SMTK_ENTITYREF_CLASS(Vertex,CellEntity,isVertex);

  Edges edges() const;

  double* coordinates() const;
  //smtk::common::Vector3d coordinates() const;
};

typedef std::vector<Vertex> Vertices;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Vertex_h
