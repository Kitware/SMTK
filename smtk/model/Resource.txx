//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_Resource_txx
#define smtk_model_Resource_txx

#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Group.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Resource.h"
#include "smtk/model/ShellEntity.txx"
#include "smtk/model/Vertex.h"

namespace smtk
{
namespace model
{

/**\brief Create face, face-use, loop, and potentially edge-use records given edges.
  *
  * Only one face use (the positive use) will be populated for the face.
  * The edges must be ordered from head to tail in a counter-clockwise loop (relative to
  * the orientation of the face use)  and the \a orderedEdgesWithOrientation
  * object passed to this method must be an STL-like container of
  * pair<smtk::model::Edge, bool> entries.
  * The boolean value stored with each edge indicates the orientation of
  * each model edge relative to the loop.
  */
template<typename T>
bool Resource::insertModelFaceWithOrientedOuterLoop(
  const smtk::common::UUID& faceId,    // to be created
  const smtk::common::UUID& faceUseId, // to be created
  const smtk::common::UUID& loopId,    // to be created
  const T& orderedEdgesWithOrientation)
{
  // First, create the ID'd top-level entity records:
  Face f = this->insertFace(faceId);
  FaceUse fu = this->insertFaceUse(faceUseId);
  Loop l = this->setLoop(loopId, fu);

  // Now loop over the container's entries (which must behave like std::pair<smtk::model::Edge, bool>)
  // adding edge uses for each edge to the loop.
  for (typename T::const_iterator oeit = orderedEdgesWithOrientation.begin();
       oeit != orderedEdgesWithOrientation.end();
       ++oeit)
  {
    smtk::model::Edge mutableEdge(oeit->first);
    EdgeUse eu = mutableEdge.findOrAddEdgeUse(oeit->second ? POSITIVE : NEGATIVE, 0);
    l.addUse(eu);
  }
  f.setFaceUse(POSITIVE, fu);
  return true;
}

/**\brief Create an inner loop (hole) in a face-use and add ordered edge uses to it.
 *
  * The edges must be ordered from head to tail in a counter-clockwise loop (relative to
  * the orientation of the face use)  and the \a orderedEdgesWithOrientation
  * object passed to this method must be an STL-like container of
  * pair<smtk::model::Edge, bool> entries.
  * The boolean value stored with each edge indicates the orientation of
  * each model edge relative to the loop.
  */
template<typename T>
bool Resource::insertModelFaceOrientedInnerLoop(
  const smtk::common::UUID& loopId,
  const smtk::common::UUID& preExistingLoopId,
  const T& orderedEdgesWithOrientation)
{
  Loop outer(shared_from_this(), preExistingLoopId);
  if (!outer.isValid())
  {
    smtkErrorMacro(
      this->log(), "Asked to add an inner loop to invalid outer loop " << outer.name());
    return false;
  }
  Loop inner = this->setLoop(loopId, outer);

  // Now loop over the container's entries (which must behave like std::pair<smtk::model::Edge, bool>)
  // adding edge uses for each edge to the loop.
  for (typename T::const_iterator oeit = orderedEdgesWithOrientation.begin();
       oeit != orderedEdgesWithOrientation.end();
       ++oeit)
  {
    smtk::model::Edge mutableEdge(oeit->first);
    EdgeUse eu = mutableEdge.findOrAddEdgeUse(oeit->second ? POSITIVE : NEGATIVE, 0);
    inner.addUse(eu);
  }
  return true;
}

/**\brief Remove free cells from a model consistently.
  *
  * This does not check whether the cells are free; it is assumed that they are.
  * However, unlike the erase() method, deleteEntities should properly reconcile
  * use records.
  *
  * So far has only been tested on 2-D models, not volumetric models.
  * It also makes assumptions about the presence of loops and edge-uses
  * (and the absence of vertex-uses and chains) that are particular to
  * the polygon session. Caveat emptor.
  */
template<typename T, typename U, typename V>
bool Resource::deleteEntities(T& entities, U& modified, V& expunged, bool debugLog)
{
  typename T::iterator eit;
  expunged.reserve(entities.size());
  typename V::value_type tmp;
  for (eit = entities.begin(); eit != entities.end(); ++eit)
  {
    smtk::model::Model mod = eit->owningModel();
    bool hadSomeEffect = false;
    smtk::model::EntityRefs bdys = eit->boundaryEntities();
    smtk::model::EntityRefs newFreeCells;
    smtk::model::Resource::Ptr mgr = eit->resource();
    if (mgr && eit->entity())
    {
      bool isCell = eit->isCellEntity();
      if (debugLog)
      {
        smtkDebugMacro(this->log(), "Erase " << eit->name() << " (c)");
      }
      tmp = eit->entityRecord();
      hadSomeEffect = (mgr->erase(*eit) != 0);
      if (hadSomeEffect)
      {
        expunged.insert(expunged.end(), tmp);
      }
      if (hadSomeEffect && isCell)
      { // Remove uses and loops "owned" by the cell
        if (debugLog)
        {
          smtkDebugMacro(this->log(), "Processing cell with " << bdys.size() << " bdys");
        }
        for (smtk::model::EntityRefs::iterator bit = bdys.begin(); bit != bdys.end(); ++bit)
        {
          if (bit->isModel())
            continue;
          if (bit->isGroup())
          {
            // Remove from group, but do not delete group.
            smtkDebugMacro(this->log(), "  Remove from group " << bit->name());
            smtk::model::Group(*bit).removeEntity(*eit);
            modified.insert(modified.end(), *bit);
            continue;
          }
          smtk::model::EntityRefs lowerShells =
            bit->as<smtk::model::UseEntity>().shellEntities<smtk::model::EntityRefs>();
          // Add child shells (inner loops):
          for (smtk::model::EntityRefs::iterator sit = lowerShells.begin();
               sit != lowerShells.end();
               ++sit)
          {
            smtk::model::EntityRefs childShells =
              sit->as<smtk::model::ShellEntity>().containedShellEntities<smtk::model::EntityRefs>();
            lowerShells.insert(childShells.begin(), childShells.end());
          }
          if (debugLog)
          {
            smtkDebugMacro(
              this->log(),
              "  Processing bdy " << bit->name() << " with " << lowerShells.size() << " shells");
          }
          for (smtk::model::EntityRefs::iterator sit = lowerShells.begin();
               sit != lowerShells.end();
               ++sit)
          {
            smtk::model::Cells bdyCells =
              sit->as<smtk::model::ShellEntity>().cellsOfUses<smtk::model::Cells>();
            if (debugLog)
            {
              smtkDebugMacro(
                this->log(),
                "    Processing shell " << sit->name() << " with " << bdyCells.size()
                                        << " bdyCells");
            }
            for (smtk::model::Cells::iterator cit = bdyCells.begin(); cit != bdyCells.end(); ++cit)
            {
              if (debugLog)
              {
                smtkDebugMacro(
                  this->log(),
                  "        Considering " << cit->name() << " as free cell: "
                                         << cit->uses<smtk::model::UseEntities>().size());
              }
              if (cit->uses<smtk::model::UseEntities>().size() <= 1)
              {
                newFreeCells.insert(*cit);
                if (debugLog)
                {
                  smtkDebugMacro(this->log(), "          Definitely a free cell ");
                }
              }
              else
              {
                if (debugLog)
                {
                  smtkDebugMacro(this->log(), "          Not a free cell ");
                }
              }
            }
            smtk::model::UseEntities bdyUses =
              sit->as<smtk::model::ShellEntity>().uses<smtk::model::UseEntities>();
            for (smtk::model::UseEntities::iterator uit = bdyUses.begin(); uit != bdyUses.end();
                 ++uit)
            {
              if (debugLog)
              {
                smtkDebugMacro(this->log(), "Erase " << uit->name() << " (su)");
              }
              expunged.insert(expunged.end(), uit->entityRecord());
              mgr->erase(*uit);
            }
            if (debugLog)
            {
              smtkDebugMacro(this->log(), "Erase " << sit->name() << " (s)");
            }
            expunged.insert(expunged.end(), sit->entityRecord());
            mgr->erase(*sit);
          }
          if (bit->isCellEntity())
          { // If the boundary entity is a direct cell relationship, see if the boundary should be promoted.
            if (debugLog)
            {
              smtkDebugMacro(
                this->log(),
                "        Considering "
                  << bit->name() << " as free cell: "
                  << bit->as<smtk::model::CellEntity>().uses<smtk::model::UseEntities>().size());
            }
            if (bit->as<smtk::model::CellEntity>().uses<smtk::model::UseEntities>().size() <= 1)
            {
              newFreeCells.insert(bit->as<smtk::model::CellEntity>());
              if (debugLog)
              {
                smtkDebugMacro(this->log(), "          Definitely a free cell ");
              }
            }
            else
            {
              if (debugLog)
              {
                smtkDebugMacro(this->log(), "          Not a free cell ");
              }
            }
          }
          else
          { // If the boundary entity is not a direct cell relationship (i.e., it's a use), erase it.
            if (debugLog)
            {
              smtkDebugMacro(this->log(), "Erase " << bit->name() << " (u)");
            }
            expunged.insert(expunged.end(), bit->entityRecord());
            mgr->erase(*bit);
          }
        }
      }
    }
    if (hadSomeEffect)
    { // Check the boundary cells of the just-removed entity and see if they are now free cells:
      for (smtk::model::EntityRefs::iterator bit = newFreeCells.begin(); bit != newFreeCells.end();
           ++bit)
      {
        if (debugLog)
        {
          smtkDebugMacro(
            this->log(), "  Adding free cell " << bit->name() << " (" << mod.cells().size() << ")");
        }
        mod.addCell(*bit);
        modified.insert(modified.end(), mod);
        if (debugLog)
        {
          smtkDebugMacro(this->log(), "    -> (" << mod.cells().size() << ")");
        }
      }
    }
  }
  return true;
}

} // namespace model
} // namespace smtk

#endif // smtk_model_Resource_txx
