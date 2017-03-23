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
#include "smtk/extension/qt/qtItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"
#include "smtk/mesh/MeshSet.h"
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
#include "smtk/extension/qt/qtModelEntityItem.h"
#include "smtk/extension/qt/qtMeshItem.h"

namespace smtk
{
  namespace extension
  {
  qtSelectionManager::qtSelectionManager()
  {
    this->clearAllSelections();
    this->m_mask = 0;
    m_mask |= smtk::model::FACE;
    m_mask |= smtk::model::EDGE;
    m_mask |= smtk::model::VERTEX;
    this->m_modelMgr = nullptr;
    this->m_selectionModifier = SelectionModifier::SELECTION_DEFAULT;
  }

  void qtSelectionManager::getSelectedEntities(smtk::common::UUIDs &selEntities)
  {
    selEntities = this->m_selEntities;
  }

  void qtSelectionManager::getSelectedMeshes(smtk::mesh::MeshSets &selMeshes)
  {
    selMeshes = this->m_selMeshes;
  }

  void qtSelectionManager::updateSelectedItems(
  const smtk::common::UUIDs &selEntities, const smtk::mesh::MeshSets &selMeshes)
  { // select from render view
    if (this->m_selectionModifier == SelectionModifier::SELECTION_DEFAULT)
    { // clear and select
      this->clearAllSelections();
      this->filterEntitySelectionsByMask(const_cast<smtk::common::UUIDs &>
                                         (selEntities), this->m_selEntities);
      this->m_selMeshes.insert(selMeshes.begin(),selMeshes.end());

    }
    else if (this->m_selectionModifier == SelectionModifier::SELECTION_ADDITION)
    { // add to current selection
      smtk::common::UUIDs currentSelFiltered;
      this->filterEntitySelectionsByMask(const_cast<smtk::common::UUIDs &>
                                         (selEntities), currentSelFiltered);
      this->m_selEntities.insert(currentSelFiltered.begin(), currentSelFiltered.end());
      this->m_selMeshes.insert(selMeshes.begin(),selMeshes.end());

    }
    else
    { //subtract from current selection
      smtk::common::UUIDs currentSelFiltered;
      this->filterEntitySelectionsByMask(const_cast<smtk::common::UUIDs &>
                                         (selEntities), currentSelFiltered);
      for (const auto& selEnt: currentSelFiltered)
      {
         this->m_selEntities.erase(selEnt);
      }

      for (const auto& selMesh: selMeshes)
      {
         this->m_selMeshes.erase(selMesh);
      }
    }

    this->m_selectionModifier = SelectionModifier::SELECTION_DEFAULT; // reset

    emit  broadcastToModelTree(this->m_selEntities,this->m_selMeshes,
                               true);
    emit broadcastToAttributeView(this->m_selEntities);
    emit broadcastToRenderView(this->m_selEntities,this->m_selMeshes,smtk::model::DescriptivePhrases());
  }

  void qtSelectionManager::updateSelectedItems(const smtk::model::EntityRefs
          &selEntities, const smtk::mesh::MeshSets &selMeshes,
                   const smtk::model::DescriptivePhrases &DesPhrases)

  { // select from model tree
    this->clearAllSelections();
    for (smtk::model::EntityRefs::iterator it = selEntities.begin();
      it != selEntities.end(); ++it)
    {
      this->m_selEntities.insert(it->entity());
    }
    this->m_selMeshes.insert(selMeshes.begin(),selMeshes.end());
    this->m_desPhrases = DesPhrases;

    emit broadcastToRenderView(selEntities, selMeshes, DesPhrases);
    emit broadcastToAttributeView(this->m_selEntities);

  }

  void qtSelectionManager::updateSelectedItems(const smtk::common::UUIDs
                                               &selEntities)
  { // select from attribute panel
    this->clearAllSelections();
    this->m_selEntities.insert(selEntities.begin(), selEntities.end());

    // broadcast to model tree and render view
    emit  broadcastToModelTree(this->m_selEntities,smtk::mesh::MeshSets(),
                               true);
    emit broadcastToRenderView(this->m_selEntities, smtk::mesh::MeshSets(),
                               smtk::model::DescriptivePhrases());
  }

  void qtSelectionManager::updateSelectedItem(const smtk::common::UUID
                                               &selEntity, int SelectionFlags)
  {
    // select from qtModelItem/operator dialog
    if ( SelectionFlags == smtk::extension::qtModelEntityItem::SelectionFlags::Add)
    {
    this->m_selEntities.insert(selEntity);
    }
    else if( SelectionFlags == smtk::extension::qtModelEntityItem::SelectionFlags::Remove)
    {
      this->m_selEntities.erase(selEntity);
    }
    else if( SelectionFlags == smtk::extension::qtModelEntityItem::SelectionFlags::Clear)
    {
      this->clearAllSelections();
    }

    // broadcast to model tree and render view
    emit  broadcastToModelTree(this->m_selEntities,smtk::mesh::MeshSets(),
                               true);
    emit broadcastToRenderView(this->m_selEntities, smtk::mesh::MeshSets(),
                               smtk::model::DescriptivePhrases());
    emit broadcastToAttributeView(this->m_selEntities);
  }

  void qtSelectionManager::filterModels(bool checked)
  {
    this->m_mask = (checked) ? (this->m_mask | smtk::model::MODEL_ENTITY)
                             : (this->m_mask & ~smtk::model::MODEL_ENTITY);
  }

  void qtSelectionManager::filterVolumes(bool checked)
  {
    // smtk::model::CELL_ENTITY makes sure that you do not affect other
    this->m_mask = (checked) ? (this->m_mask | smtk::model::VOLUME)
                : (m_mask & (~smtk::model::VOLUME | smtk::model::CELL_ENTITY));
  }

  void qtSelectionManager::filterFaces(bool checked)
  {
    // smtk::model::CELL_ENTITY makes sure that you do not affect other
    this->m_mask = (checked) ? (this->m_mask | smtk::model::FACE)
                : (m_mask & (~smtk::model::FACE | smtk::model::CELL_ENTITY));
  }

  void qtSelectionManager::filterEdges(bool checked)
  {
    // smtk::model::CELL_ENTITY makes sure that you do not affect other
    this->m_mask = (checked) ? (this->m_mask | smtk::model::EDGE)
                : (m_mask & (~smtk::model::EDGE | smtk::model::CELL_ENTITY));
  }

  void qtSelectionManager::filterVertices(bool checked)
  {
    // smtk::model::CELL_ENTITY makes sure that you do not affect other
    this->m_mask = (checked) ? (this->m_mask | smtk::model::VERTEX)
                : (m_mask & (~smtk::model::VERTEX | smtk::model::CELL_ENTITY));
  }

  void qtSelectionManager::clearAllSelections()
  {
    this->m_selEntities.clear();
    this->m_selMeshes.clear();
    this->m_desPhrases.clear();
  }

  void qtSelectionManager::filterEntitySelectionsByMask(
      smtk::common::UUIDs &currentSelEnt, smtk::common::UUIDs &filteredSelEnt)
  {
    filteredSelEnt.clear();
    // For now rubber band selection only support F/E/V
    for(smtk::common::UUIDs::iterator uuid = currentSelEnt.begin(); uuid != currentSelEnt.end(); uuid++)
    {
      smtk::model::EntityRef ent = smtk::model::EntityRef(this->m_modelMgr, *uuid);
      if (this->m_mask & smtk::model::CELL_ENTITY)
      {
        // check Cell? dimension? match mask?
        if ((ent.entityFlags() & smtk::model::CELL_ENTITY) &&
             ((ent.entityFlags() & smtk::model::ANY_DIMENSION) & this->m_mask))
        {
          filteredSelEnt.insert(*uuid);
        }
      }

      if (ent.entityFlags() & smtk::model::GROUP_CONSTRAINT_MASK)
      {
        filteredSelEnt.insert(*uuid);
      }

      // Comment out for now since tessellation for volume and model is not added
      //if (this->m_mask & smtk::model::MODEL_ENTITY)
      //{
      //  this->m_selEntities.insert(ent.owningModel().entity());
      //}

      //// check only volume condition
      //if (this->m_mask & ~smtk::model::CELL_ENTITY & smtk::model::VOLUME)
      //{
      //  if (ent.isFace())
      //  {
      //    smtk::model::Face face = ent.as<smtk::model::Face>();
      //    smtk::model::Volumes volumesFa = face.volumes();
      //    for (const auto & volume : volumesFa)
      //    {
      //      this->m_selEntities.insert(volume.entity());
      //    }
      //  }
      //  else if (ent.isEdge())
      //  {
      //    smtk::model::Edge edge = ent.as<smtk::model::Edge>();
      //    smtk::model::Faces facesEd = edge.faces();
      //    for (const auto & faceEd: facesEd)
      //    {
      //      smtk::model::Volumes volumesFaEd = faceEd.volumes();
      //      for (const auto & volumeFaEd : volumesFaEd)
      //      {
      //        this->m_selEntities.insert(volumeFaEd.entity());
      //      }
      //    }
      //  }
      //  else if (ent.isVertex())
      //  {
      //    smtk::model::Vertex vertex = ent.as<smtk::model::Vertex>();
      //    smtk::model::Edges edgesVe = vertex.edges();
      //    for (const auto & edgeVe : edgesVe)
      //    {
      //      smtk::model::Faces facesEdVe = edgeVe.faces();
      //      for (const auto & faceEdVe : facesEdVe)
      //      {
      //        smtk::model::Volumes volumesFaEdVe = faceEdVe.volumes();
      //        for (const auto & volumeFaEdVe : volumesFaEdVe)
      //        {
      //          this->m_selEntities.insert(volumeFaEdVe.entity());
      //        }
      //      }
      //    }
      //  }

      //}
    }
  }

  }; // namespace extension

}; // namespace smtk

