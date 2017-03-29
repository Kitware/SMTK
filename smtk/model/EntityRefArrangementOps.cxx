//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/EntityRefArrangementOps.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"

namespace smtk {
  namespace model {

/**\brief Return the index of a relationship between \a a and \a b if it exists.
  *
  */
int EntityRefArrangementOps::findSimpleRelationship(
  const EntityRef& a, ArrangementKind k, const EntityRef& b)
{
  int na = a.numberOfArrangementsOfKind(k);
  for (int i = 0; i < na; ++i)
    {
    if (a.relationFromArrangement(k, i, 0) == b)
      {
      return i;
      }
    }
  return -1;
}

/**\brief Create the relationship between \a a and \a b (unless it already exists).
  *
  * In either event, return its index.
  */
int EntityRefArrangementOps::findOrAddSimpleRelationship(
  const EntityRef& a, ArrangementKind k, const EntityRef& b)
{
  int relidx = EntityRefArrangementOps::findSimpleRelationship(a, k, b);
  if (relidx < 0)
    {
    Entity* ent = a.manager()->findEntity(a.entity());
    if (ent)
      {
      int offset = ent->findOrAppendRelation(b.entity());
      relidx = a.manager()->arrangeEntity(
        a.entity(), k, Arrangement::SimpleIndex(offset));
      }
    else
      {
      relidx = -1;
      }
    }
  return relidx;
}

  } // namespace model
} // namespace smtk
