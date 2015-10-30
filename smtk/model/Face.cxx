//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Face.h"

#include "smtk/model/Edge.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Volume.h"

namespace smtk {
  namespace model {

smtk::model::Edges Face::edges() const
{
  Edges result;
  EntityRefs all = this->boundaryEntities(/*dim = */ 1);
  for (EntityRefs::iterator it = all.begin(); it != all.end(); ++it)
    {
    if (it->isEdge())
      result.push_back(*it);
    }
  return result;
}

smtk::model::Volumes Face::volumes() const
{
  Volumes result;
  EntityRefs all = this->bordantEntities(/*dim = */ 3);
  for (EntityRefs::iterator it = all.begin(); it != all.end(); ++it)
    {
    if (it->isVolume())
      result.push_back(*it);
    }
  return result;
}

/**\brief Return the face-use with its sense opposite the natural normal.
  *
  * This may return an invalid entry if no such use exists.
  */
FaceUse Face::negativeUse() const
{
  ManagerPtr mgr = this->manager();
  std::set<int> arr = mgr->findCellHasUsesWithOrientation(
    this->m_entity, NEGATIVE);
  return arr.empty() ?
    FaceUse() :
    relationFromArrangement(HAS_USE, *arr.begin(), 0).as<FaceUse>();
}

/**\brief Return the face-use with its sense codirectional with the natural normal.
  *
  * This may return an invalid entry if no such use exists.
  */
FaceUse Face::positiveUse() const
{
  ManagerPtr mgr = this->manager();
  std::set<int> arr = mgr->findCellHasUsesWithOrientation(
    this->m_entity, POSITIVE);
  return arr.empty() ?
    FaceUse() :
    relationFromArrangement(HAS_USE, *arr.begin(), 0).as<FaceUse>();
}

/**\brief Add or replace any existing face use with the given use-record \a u.
  *
  */
void Face::setFaceUse(Orientation orientation, const FaceUse& u)
{
  smtk::model::Manager::Ptr mgr = this->manager();
  if (this->isValid())
    {
    mgr->findCreateOrReplaceCellUseOfSenseAndOrientation(this->m_entity, 0, orientation, u.entity());
    }
}

  } // namespace model
} // namespace smtk
