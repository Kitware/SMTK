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
#include "smtk/model/Resource.h"

namespace smtk
{
namespace model
{

/**\brief Return the index of a relationship between \a a and \a b if it exists.
  *
  */
int EntityRefArrangementOps::findSimpleRelationship(
  const EntityRef& a,
  ArrangementKind k,
  const EntityRef& b)
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
  const EntityRef& a,
  ArrangementKind k,
  const EntityRef& b)
{
  int relidx = EntityRefArrangementOps::findSimpleRelationship(a, k, b);
  if (relidx < 0)
  {
    relidx = EntityRefArrangementOps::addSimpleRelationship(a, k, b);
  }
  return relidx;
}

/**\brief Create the relationship between \a a and \a b (whether or not it already exists).
  *
  * This method returns the index of the relationship added or -1
  * if either of the entities do not exist.
  */
int EntityRefArrangementOps::addSimpleRelationship(
  const EntityRef& a,
  ArrangementKind k,
  const EntityRef& b,
  bool find)
{
  int relidx = -1;
  EntityPtr ent = a.resource()->findEntity(a.entity());
  if (ent)
  {
    int offset = find ? ent->findOrAppendRelation(b.entity()) : ent->appendRelation(b.entity());
    relidx = a.resource()->arrangeEntity(a.entity(), k, Arrangement::SimpleIndex(offset));
  }
  return relidx;
}

} // namespace model
} // namespace smtk
