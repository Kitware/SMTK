//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/operators/Delete.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"

#include "smtk/io/Logger.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Face.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/ShellEntity.txx"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/Delete_xml.h"

#define smtkOpDebug(x) \
  if (this->m_debugLevel > 0) \
    { \
    smtkDebugMacro(this->log(), x ); \
    }

namespace smtk {
  namespace bridge {
    namespace polygon {

smtk::model::OperatorResult Delete::operateInternal()
{
  smtk::attribute::VoidItem::Ptr deleteDependentItem =
    this->findVoid("delete dependents");
  bool deleteDependents = deleteDependentItem->isEnabled();

  smtkOpDebug("associations: " << this->specification()->associations()->numberOfValues());
  smtk::model::EntityRefs entities = this->associatedEntitiesAs<smtk::model::EntityRefs>();
  /*
  smtk::model::VertexSet verts = this->associatedEntitiesAs<smtk::model::VertexSet>();
  smtk::model::EdgeSet edges = this->associatedEntitiesAs<smtk::model::EdgeSet>();
  smtk::model::FaceSet faces = this->associatedEntitiesAs<smtk::model::FaceSet>();
  std::cout
    << "  of which "
    << faces.size() << " faces, "
    << edges.size() << " edges, and "
    << verts.size() << " verts were accepted.";
    */

  int cannotDeleteCount = 0;
  smtk::model::EntityRefs::iterator eit;
  smtk::model::EntityRefs addMe;
  if (!deleteDependents)
    {
    // Get a count of how many entities cannot be deleted.
    for (eit = entities.begin(); eit != entities.end(); ++eit)
      {
      smtk::model::EntityRef mutableEntity(*eit);
      smtk::model::EntityRefs brd = mutableEntity.bordantEntities();
      smtkOpDebug("Consider " << eit->name() << " db " << eit->dimensionBits());
      // Compute the dimension we want to reject -- because bordantEntities() will
      // report use-records and models (for free cells) that are "border-like" but
      // should not prevent deletion:
      int higherDim = (eit->dimensionBits() << 1);
      for (smtk::model::EntityRefs::iterator bit = brd.begin(); bit != brd.end(); ++bit)
        {
        smtkOpDebug("  bordant " << bit->name() << " db " << eit->dimensionBits());
        if (entities.find(*bit) == entities.end())
          {
          if ((bit->dimensionBits() >= higherDim) ||
            !bit->as<smtk::model::UseEntity>().boundingShellEntities<smtk::model::EntityRefArray>().empty())
            {
            ++cannotDeleteCount;
            break;
            }
          }
        }
      }
    }
  else
    {
    // Get a list of dependent entities to delete.
    for (eit = entities.begin(); eit != entities.end(); ++eit)
      {
      smtk::model::EntityRef mutableEntity(*eit);
      smtk::model::EntityRefs brd = mutableEntity.higherDimensionalBordants(-1);
      smtk::model::EntityRefArray bc;
      int higherDim = (eit->dimensionBits() << 1);
      smtkOpDebug("Consider cell " << eit->name() << " bordants " << brd.size() << " dimBits " << higherDim);
      for (smtk::model::EntityRefs::iterator bit = brd.begin(); bit != brd.end(); ++bit)
        {
        smtkOpDebug("Consider bdy " << bit->name() << " of " << eit->name());
        if (bit->dimensionBits() >= higherDim)
          { // the cell directly reference cells of higher dimension... add those cells:
          smtkOpDebug("  Adding brd " << bit->name() << " to kill list");
          addMe.insert(*bit);
          }
        else if (!(bc = bit->as<smtk::model::UseEntity>().boundingShellEntities<smtk::model::EntityRefArray>()).empty())
          { // the cell references uses whose higher-dimensional shell(s) reference use-records whose cells we want to add:
          smtkOpDebug("  Brd has " << bc.size() << " entries...");
          for (smtk::model::EntityRefArray::iterator bcit = bc.begin(); bcit != bc.end(); ++bcit)
            {
            smtkOpDebug("  Consider " << bcit->name());
            smtk::model::CellEntity cell = bcit->as<smtk::model::CellEntity>();
            if (cell.isValid())
              {
              smtkOpDebug("  Adding dependent cell " << cell.name());
              addMe.insert(cell);
              }
            }
          }
        }
      }
    }
  entities.insert(addMe.begin(), addMe.end());

  if (cannotDeleteCount > 0)
    {
    smtkErrorMacro(this->log(),
      "Cannot delete " << cannotDeleteCount
      << " of " << entities.size()
      << " entities as they are in use.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::EntityRefs notRemoved;
  smtk::model::EntityRefs modified;
  smtk::model::EntityRefArray expunged;
  expunged.reserve(entities.size());
  for (eit = entities.begin(); eit != entities.end(); ++eit)
    {
    smtk::model::Model mod = eit->owningModel();
    bool hadSomeEffect = false;
    smtk::model::EntityRefs bdys = eit->boundaryEntities();
    smtk::model::EntityRefs newFreeCells;
    smtk::model::Manager::Ptr mgr = eit->manager();
    if (mgr && eit->entity())
      {
      bool isCell = eit->isCellEntity();
      smtkOpDebug("Erase " << eit->name() << " (c)");
      hadSomeEffect = (mgr->erase(*eit) != 0);
      if (hadSomeEffect && isCell)
        { // Remove uses and loops "owned" by the cell
        expunged.push_back(*eit);
        smtkOpDebug("Processing cell with " << bdys.size() << " bdys");
        for (smtk::model::EntityRefs::iterator bit = bdys.begin(); bit != bdys.end(); ++bit)
          {
          if (bit->isModel())
            continue;
          smtk::model::EntityRefs lowerShells =
            bit->as<smtk::model::UseEntity>().shellEntities<smtk::model::EntityRefs>();
          // Add child shells (inner loops):
          for (smtk::model::EntityRefs::iterator sit = lowerShells.begin(); sit != lowerShells.end(); ++sit)
            {
            smtk::model::EntityRefs childShells =
              sit->as<smtk::model::ShellEntity>().containedShellEntities<smtk::model::EntityRefs>();
            lowerShells.insert(childShells.begin(), childShells.end());
            }
          smtkOpDebug("  Processing bdy " << bit->name() << " with " << lowerShells.size() << " shells");
          for (smtk::model::EntityRefs::iterator sit = lowerShells.begin(); sit != lowerShells.end(); ++sit)
            {
            smtk::model::Cells bdyCells = sit->as<smtk::model::ShellEntity>().cellsOfUses<smtk::model::Cells>();
            smtkOpDebug("    Processing shell " << sit->name() << " with " << bdyCells.size() << " bdyCells");
            for (smtk::model::Cells::iterator cit = bdyCells.begin(); cit != bdyCells.end(); ++cit)
              {
              smtkOpDebug(
                  "        Considering " << cit->name() << " as free cell: "
                  << cit->uses<smtk::model::UseEntities>().size());
              if (cit->uses<smtk::model::UseEntities>().size() <= 1)
                {
                newFreeCells.insert(*cit);
                smtkOpDebug("          Definitely a free cell ");
                }
              else
                {
                smtkOpDebug("          Not a free cell ");
                }
              }
            smtk::model::UseEntities bdyUses = sit->as<smtk::model::ShellEntity>().uses<smtk::model::UseEntities>();
            for (smtk::model::UseEntities::iterator uit = bdyUses.begin(); uit != bdyUses.end(); ++uit)
              {
              smtkOpDebug("Erase " << uit->name() << " (su)");
              mgr->erase(*uit);
              }
            smtkOpDebug("Erase " << sit->name() << " (s)");
            mgr->erase(*sit);
            }
          smtkOpDebug("Erase " << bit->name() << " (u)");
          mgr->erase(*bit);
          }
        }
      }
    hadSomeEffect |= this->removeStorage(eit->entity());
    if (hadSomeEffect)
      { // Check the boundary cells of the just-removed entity and see if they are now free cells:
      for (smtk::model::EntityRefs::iterator bit = newFreeCells.begin(); bit != newFreeCells.end(); ++bit)
        {
        smtkOpDebug("  Adding free cell " << bit->name() << " (" << mod.cells().size() << ")");
        mod.addCell(*bit);
        modified.insert(mod);
        smtkOpDebug("    -> (" << mod.cells().size() << ")");
        }
      }
    else
      {
      notRemoved.insert(*eit);
      }
    }

  smtk::model::OperatorResult result =
    this->createResult(smtk::model::OPERATION_SUCCEEDED);
  this->addEntitiesToResult(result, expunged, EXPUNGED);
  smtk::model::EntityRefArray modray(modified.begin(), modified.end());
  this->addEntitiesToResult(result, modray, MODIFIED);

  smtkInfoMacro(this->log(),
    "Deleted " << expunged.size() << " of " << entities.size() << " requested entities");

  return result;
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  SMTKPOLYGONSESSION_EXPORT,
  smtk::bridge::polygon::Delete,
  polygon_delete,
  "delete",
  Delete_xml,
  smtk::bridge::polygon::Session);
