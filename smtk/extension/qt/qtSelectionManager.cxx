//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtSelectionManager.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"
#include "smtk/extension/qt/qtMeshItem.h"
#include "smtk/extension/qt/qtModelEntityItem.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtk/model/Face.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/Volume.h"

namespace smtk
{
namespace extension
{
qtSelectionManager::qtSelectionManager()
{
  this->clear();
  this->m_mask = 0;
  m_mask |= smtk::model::FACE;
  m_mask |= smtk::model::EDGE;
  m_mask |= smtk::model::VERTEX;
  this->m_filterMeshes = false;
  this->m_modelMgr = nullptr;
  this->m_selEntityRefs = smtk::model::EntityRefs();
  this->m_selMeshes = smtk::mesh::MeshSets();
  this->m_selectionAction = SelectionAction::FILTERED_REPLACE;
}

void qtSelectionManager::getSelectedEntities(smtk::common::UUIDs& selEntities)
{
  for (const auto& selEnt : this->m_selEntityRefs)
  {
    selEntities.insert(selEnt.entity());
  }
}

void qtSelectionManager::getSelectedEntitiesAsEntityRefs(smtk::model::EntityRefs& selEntities)
{
  selEntities = this->m_selEntityRefs;
}

void qtSelectionManager::getSelectedMeshes(smtk::mesh::MeshSets& selMeshes)
{
  selMeshes = this->m_selMeshes;
}

void qtSelectionManager::getSelectionSources(std::set<std::string>& selectionSources)
{
  selectionSources = this->m_selectionSources;
}

void qtSelectionManager::updateSelectedItems(const smtk::model::EntityRefs& selEntities,
  const smtk::mesh::MeshSets& selMeshes, const smtk::model::DescriptivePhrases& /*DesPhrases*/,
  const smtk::view::SelectionAction actionFlag, const std::string& incomingSelectionSource)
{
  if (actionFlag == smtk::view::SelectionAction::UNFILTERED_ADD)
  { // \b selection from qtModelItem/operator dialog
    this->m_selEntityRefs.insert(selEntities.begin(), selEntities.end());
    this->m_selMeshes.insert(selMeshes.begin(), selMeshes.end());
  }
  else if (actionFlag == smtk::view::SelectionAction::UNFILTERED_SUBTRACT)
  { // \b selection from qtModelItem/operator dialog
    for (auto selEntity : selEntities)
    {
      if (this->m_selEntityRefs.find(selEntity) != this->m_selEntityRefs.end())
      {
        this->m_selEntityRefs.erase(selEntity);
      }
    }
    this->m_selMeshes.erase(selMeshes.begin(), selMeshes.end());
  }
  else if (actionFlag == smtk::view::SelectionAction::UNFILTERED_REPLACE)
  {
    // \b clear selection in qtModelItem/opeartor dialog
    // \b selection from model tree
    // \b selection from attribute panel

    // only store selection of the active model
    smtk::model::EntityRefs filteredSelEnts;
    for (const auto& selEnt : selEntities)
    {
      if ((selEnt.owningModel().entity() == qtActiveObjects::instance().activeModel().entity()) ||
        (selEnt.entity() == qtActiveObjects::instance().activeModel().entity()) ||
        (selEnt.owningModel().parent().entity() ==
            qtActiveObjects::instance().activeModel().entity()) ||
        selEnt.isSessionRef())
      {
        filteredSelEnts.insert(selEnt);
      }
    }
    this->clear();
    this->m_selEntityRefs.insert(filteredSelEnts.begin(), filteredSelEnts.end());
    this->m_selMeshes.insert(selMeshes.begin(), selMeshes.end());
  }
  else if (actionFlag == smtk::view::SelectionAction::DEFAULT)
  { // \b selection from render window
    if (this->m_selectionAction == SelectionAction::FILTERED_REPLACE)
    { // clear and select
      this->clear();
      this->filterEntitySelectionsByMaskAndActiveModel(
        const_cast<smtk::model::EntityRefs&>(selEntities), this->m_selEntityRefs);
      if (this->m_filterMeshes)
      {
        this->m_selMeshes.insert(selMeshes.begin(), selMeshes.end());
      }
    }
    else if (this->m_selectionAction == SelectionAction::FILTERED_ADD)
    { // add to current selection
      smtk::model::EntityRefs currentSelFiltered;
      this->filterEntitySelectionsByMaskAndActiveModel(
        const_cast<smtk::model::EntityRefs&>(selEntities), currentSelFiltered);
      this->m_selEntityRefs.insert(currentSelFiltered.begin(), currentSelFiltered.end());

      if (this->m_filterMeshes)
      {
        this->m_selMeshes.insert(selMeshes.begin(), selMeshes.end());
      }
    }
    else if (this->m_selectionAction == SelectionAction::FILTERED_SUBTRACT)
    { //subtract from current selection
      smtk::model::EntityRefs currentSelFiltered;
      this->filterEntitySelectionsByMaskAndActiveModel(
        const_cast<smtk::model::EntityRefs&>(selEntities), currentSelFiltered);
      for (const auto& selEnt : currentSelFiltered)
      {
        this->m_selEntityRefs.erase(selEnt);
      }

      if (this->m_filterMeshes)
      {
        for (const auto& selMesh : selMeshes)
        {
          this->m_selMeshes.erase(selMesh);
        }
      }
    }

    this->m_selectionAction = SelectionAction::FILTERED_REPLACE; // reset
  }

  // broadcast to rendering view, model tree and attribute panel if needed
  emit broadcastToReceivers(
    this->m_selEntityRefs, this->m_selMeshes, this->m_desPhrases, incomingSelectionSource);
}

void qtSelectionManager::filterModels(bool checked)
{
  this->m_mask = (checked) ? (this->m_mask | smtk::model::MODEL_ENTITY)
                           : (this->m_mask & ~smtk::model::MODEL_ENTITY);
}

void qtSelectionManager::filterVolumes(bool checked)
{
  // smtk::model::CELL_ENTITY makes sure that you do not affect each other
  this->m_mask = (checked) ? (this->m_mask | smtk::model::VOLUME)
                           : (m_mask & (~smtk::model::VOLUME | smtk::model::CELL_ENTITY));
}

void qtSelectionManager::filterFaces(bool checked)
{
  // smtk::model::CELL_ENTITY makes sure that you do not affect each other
  this->m_mask = (checked) ? (this->m_mask | smtk::model::FACE)
                           : (m_mask & (~smtk::model::FACE | smtk::model::CELL_ENTITY));
}

void qtSelectionManager::filterEdges(bool checked)
{
  // smtk::model::CELL_ENTITY makes sure that you do not affect each other
  this->m_mask = (checked) ? (this->m_mask | smtk::model::EDGE)
                           : (m_mask & (~smtk::model::EDGE | smtk::model::CELL_ENTITY));
}

void qtSelectionManager::filterVertices(bool checked)
{
  // smtk::model::CELL_ENTITY makes sure that you do not affect each other
  this->m_mask = (checked) ? (this->m_mask | smtk::model::VERTEX)
                           : (m_mask & (~smtk::model::VERTEX | smtk::model::CELL_ENTITY));
}

void qtSelectionManager::filterMeshes(bool checked)
{
  this->m_filterMeshes = checked;
}

void qtSelectionManager::clearAllSelections()
{
  this->updateSelectedItems(smtk::model::EntityRefs(), smtk::mesh::MeshSets(),
    smtk::model::DescriptivePhrases(), smtk::view::SelectionAction::UNFILTERED_REPLACE,
    std::string());
}

void qtSelectionManager::clear()
{
  this->m_selEntityRefs.clear();
  this->m_selMeshes.clear();
  this->m_desPhrases.clear();
}

void qtSelectionManager::filterEntitySelectionsByMaskAndActiveModel(
  smtk::model::EntityRefs& inputEnts, smtk::model::EntityRefs& filteredSelEnts)
{
  filteredSelEnts.clear();
  // For now rubber band selection only support F/E/V and group
  for (smtk::model::EntityRefs::iterator inputEnt = inputEnts.begin(); inputEnt != inputEnts.end();
       inputEnt++)
  {
    smtk::model::EntityRef ent = *inputEnt;

    // only filter active model's item
    if ((ent.owningModel().entity() != qtActiveObjects::instance().activeModel().entity()) &&
      (ent.isModel() && ent.entity() != qtActiveObjects::instance().activeModel().entity()) &&
      (ent.owningModel().parent().isValid() &&
          ent.owningModel().parent().entity() !=
            qtActiveObjects::instance().activeModel().entity()))
    {
      continue;
    }
    if (this->m_mask & smtk::model::CELL_ENTITY)
    {
      // check Cell? dimension? match mask?
      if ((ent.entityFlags() & smtk::model::CELL_ENTITY) &&
        ((ent.entityFlags() & smtk::model::ANY_DIMENSION) & this->m_mask))
      {
        filteredSelEnts.insert(ent);
      }
    }

    if (ent.entityFlags() & smtk::model::GROUP_CONSTRAINT_MASK)
    {
      filteredSelEnts.insert(ent);
    }

    if (this->m_mask & smtk::model::MODEL_ENTITY)
    {
      filteredSelEnts.insert(ent.owningModel());
      // Add top level model
      if (ent.owningModel().parent().isValid() && ent.owningModel().parent().isModel())
      {
        filteredSelEnts.insert(ent.owningModel().parent());
      }
    }

    // check only volume condition
    if (this->m_mask & ~smtk::model::CELL_ENTITY & smtk::model::VOLUME)
    {
      if (ent.isFace())
      {
        smtk::model::Face face = ent.as<smtk::model::Face>();
        smtk::model::Volumes volumesFa = face.volumes();
        for (const auto& volume : volumesFa)
        {
          filteredSelEnts.insert(volume);
        }
      }
      else if (ent.isEdge())
      {
        smtk::model::Edge edge = ent.as<smtk::model::Edge>();
        smtk::model::Faces facesEd = edge.faces();
        for (const auto& faceEd : facesEd)
        {
          smtk::model::Volumes volumesFaEd = faceEd.volumes();
          for (const auto& volumeFaEd : volumesFaEd)
          {
            filteredSelEnts.insert(volumeFaEd);
          }
        }
      }
      else if (ent.isVertex())
      {
        smtk::model::Vertex vertex = ent.as<smtk::model::Vertex>();
        smtk::model::Edges edgesVe = vertex.edges();
        for (const auto& edgeVe : edgesVe)
        {
          smtk::model::Faces facesEdVe = edgeVe.faces();
          for (const auto& faceEdVe : facesEdVe)
          {
            smtk::model::Volumes volumesFaEdVe = faceEdVe.volumes();
            for (const auto& volumeFaEdVe : volumesFaEdVe)
            {
              filteredSelEnts.insert(volumeFaEdVe);
            }
          }
        }
      }
    }
  }
}

}; // namespace extension

}; // namespace smtk
