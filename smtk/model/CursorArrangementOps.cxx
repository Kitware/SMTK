#include "smtk/model/CursorArrangementOps.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/Cursor.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Storage.h"

namespace smtk {
  namespace model {

/**\brief Return the index of a relationship between \a a and \a b if it exists.
  *
  */
int CursorArrangementOps::findSimpleRelationship(
  const Cursor& a, ArrangementKind k, const Cursor& b)
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
int CursorArrangementOps::findOrAddSimpleRelationship(
  const Cursor& a, ArrangementKind k, const Cursor& b)
{
  int relidx = CursorArrangementOps::findSimpleRelationship(a, k, b);
  if (relidx < 0)
    {
    Entity* ent = a.storage()->findEntity(a.entity());
    if (ent)
      {
      int offset = ent->findOrAppendRelation(b.entity());
      relidx = a.storage()->arrangeEntity(
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
