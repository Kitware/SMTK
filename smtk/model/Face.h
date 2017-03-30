//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Face_h
#define __smtk_model_Face_h

#include "smtk/model/Arrangement.h" // for ArrangementReference
#include "smtk/model/CellEntity.h"

//#include "smtk/common/Eigen.h" // For Vector3d

#include <vector>

namespace smtk
{
namespace model
{

class Edge;
class FaceUse;
class Loop;
class Volume;
typedef std::vector<Edge> Edges;
typedef std::vector<FaceUse> FaceUses;
typedef std::vector<Volume> Volumes;

/**\brief A entityref subclass that provides methods specific to 2-d face cells.
  *
  */
class SMTKCORE_EXPORT Face : public CellEntity
{
public:
  SMTK_ENTITYREF_CLASS(Face, CellEntity, isFace);

  Edges edges() const;
  Volumes volumes() const;
  FaceUse negativeUse() const;
  FaceUse positiveUse() const;

  void setFaceUse(Orientation o, const FaceUse& u);
};

} // namespace model
} // namespace smtk

#endif // __smtk_model_Face_h
