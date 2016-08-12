//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Loop.h"

#include "smtk/model/EntityRefArrangementOps.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Manager.h"

#include "smtk/common/RangeDetector.h"

#include <list>

namespace smtk {
  namespace model {

/**\brief Return the high-dimensional cell whose interior is bounded by this shell.
  *
  */
Face Loop::face() const
{
  return this->ShellEntity::boundingCell().as<Face>();
}

/**\brief Return the face-use bounded by this loop.
  *
  */
FaceUse Loop::faceUse() const
{
  return this->ShellEntity::boundingUseEntity().as<FaceUse>();
}

/**\brief Return the edge-uses composing this loop.
  *
  * The edge uses are properly ordered and co-directional
  * with the sense of the loop.
  */
EdgeUses Loop::edgeUses() const
{
  return this->ShellEntity::uses<EdgeUses>();
}

/**\brief Return the parent shell of this shell (or an invalid shell if unbounded).
  *
  */
Loop Loop::containingLoop() const
{
  return this->ShellEntity::containingShellEntity().as<Loop>();
}

/**\brief Return the child shells of this shell, if any.
  *
  */
Loops Loop::containedLoops() const
{
  return this->ShellEntity::containedShellEntities<Loops>();
}

/**\brief Replace one edge use in a loop with 0 or more edge-uses.
  *
  * The replacement conserves edge-use order as required for loops.
  * This can be used (once for each loop-use) to update records when splitting an edge.
  * It can also be used to remove an edge from a use when merging (by replacing
  * the \a original with an empty \a replacements array).
  */
bool Loop::replaceEdgeUseWithUses(const EdgeUse& original, const EdgeUses& replacements)
{
  Manager::Ptr mgr = this->manager(); // Lock mgr
  Entity* entRec = mgr->findEntity(this->entity());
  if (!entRec)
    {
    return false;
    }
  smtk::common::UUIDArray::iterator loc =
    std::find(entRec->relations().begin(), entRec->relations().end(),
      original.entity());
  if (loc == entRec->relations().end())
    { // original was not in the loop!
    return false;
    }
  Arrangements* arr = mgr->hasArrangementsOfKindForEntity(this->entity(), HAS_USE);
  if (!arr)
    { // no HAS_USE arrangements means \a original is not present as an arrangement...
    return false;
    }
  std::list<EdgeUse> uses;
  smtk::common::RangeDetector<int> rd;
  // Get all of the existing uses of this shell. At the same time, this will wipe away
  // all HAS_USE arrangements **and** invalidate the relations of the loop:
  EntityRefArrangementOps::popAllShellHasUseRelations(mgr, entRec, arr, uses, rd);

  std::list<EdgeUse>::iterator uit = std::find(uses.begin(), uses.end(), original);
  bool didFind;
  if (uit == uses.end())
    {
    didFind = false;
    smtkErrorMacro(mgr->log(),
      "Found use-record " << original.name() << " in loop but could not replace it.");
    }
  else
    { // Insert the replacements and remove the original:
    didFind = true;
    uses.insert(uit, replacements.begin(), replacements.end());
    uses.erase(uit);
    }

  // Now add the updated uses back to both the relations (in the order RangeDetector provides)
  // and as arrangements (one per range).

  std::map<int,int>::iterator rdit;
  uit = uses.begin();
  size_t nr;
  // Add relations as required to handle all of the use-records
  smtk::common::UUID dummy;
  while ((nr = rd.size()) < uses.size())
    {
    rd.insert(
      entRec->appendRelation(dummy, true));
    }

  for (rdit = rd.ranges().begin(); rdit != rd.ranges().end(); ++rdit)
    {
    int ii;
    for (ii = rdit->first; ii <= rdit->second && uit != uses.end(); ++ii, ++uit)
      {
      entRec->relations()[ii] = uit->entity();
      }
    arr->push_back(Arrangement::ShellHasUseWithIndexRange(rdit->first, ii - 1));
    }

  // The above created a new arrangement for the Loop... now we need to
  // (1) remove the original's reference to the loop and
  // (2) add new references from the replacements to the loop.
  int aidx = mgr->findArrangementInvolvingEntity(original.entity(), HAS_SHELL, this->m_entity);
  //std::cout << "Original idx " << aidx << "\n";
  int didRemove = mgr->unarrangeEntity(original.entity(), HAS_SHELL, aidx, false);
  //std::cout << "Did remove original " << didRemove << "\n";
  for (EdgeUses::const_iterator rit = replacements.begin(); rit != replacements.end(); ++rit)
    {
    Entity* useRec = mgr->findEntity(rit->entity());
    if (!useRec)
      {
      continue;
      }
    mgr->arrangeEntity(rit->entity(), HAS_SHELL, Arrangement::UseHasShellWithIndex(
        useRec->appendRelation(this->m_entity)), -1);
    }

  return didFind ? true : false;
}

  } // namespace model
} // namespace smtk
