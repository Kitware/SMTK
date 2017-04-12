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
    this->m_filterMeshes = true;
    this->m_modelMgr = nullptr;
    this->m_selectionModifier = SelectionModifier::SELECTION_REPLACE_FILTERED;
    this->m_skipList.push_back(std::string("rendering window"));
    this->m_skipList.push_back(std::string("model tree"));
    this->m_skipList.push_back(std::string("attribute panel"));
  }

  void qtSelectionManager::getSelectedEntities(smtk::common::UUIDs &selEntities)
  {
    selEntities = this->m_selEntities;
  }

  void qtSelectionManager::getSelectedEntitiesAsEntityRefs(smtk::model::EntityRefs &selEntities)
  {
    selEntities = this->m_selEntityRefs;
  }

  void qtSelectionManager::getSelectedMeshes(smtk::mesh::MeshSets &selMeshes)
  {
    selMeshes = this->m_selMeshes;
  }

  void qtSelectionManager::updateSelectedItems(
                            const smtk::model::EntityRefs &selEntities,
                            const smtk::mesh::MeshSets &selMeshes,
                            const smtk::model::DescriptivePhrases &/*DesPhrases*/,
                            const smtk::extension::SelectionModifier modifierFlag,
                            const smtk::model::StringList skipList
                            )
  {
    // \b selection from qtModelItem/operator dialog
    if ( modifierFlag == smtk::extension::SelectionModifier::SELECTION_ADDITION_UNFILTERED)
    {
      this->m_selEntityRefs.insert(selEntities.begin(), selEntities.end());
      this->m_selMeshes.insert(selMeshes.begin(),selMeshes.end());
      // Deprecated start
      for (auto selEntity: selEntities)
      {
      this->m_selEntities.insert(selEntity.entity());
      }
      // Deprecated end
    }
    else if ( modifierFlag == smtk::extension::SelectionModifier::SELECTION_SUBTRACTION_UNFILTERED)
    {
      this->m_selEntityRefs.erase(selEntities.begin(), selEntities.end());
      this->m_selMeshes.insert(selMeshes.begin(),selMeshes.end());
      // Deprecated start
      for (auto selEntity: selEntities)
      {
      this->m_selEntities.erase(selEntity.entity());
      }
      // Deprecated end
    }
    else if ( modifierFlag == smtk::extension::SelectionModifier::SELECTION_REPLACE_UNFILTERED)
    {
      // \b clear selection in qtModelItem/opeartor dialog
      // \b selection from model tree
      // \b selection from  attribue panel
      this->clearAllSelections();
      this->m_selEntityRefs.insert(selEntities.begin(), selEntities.end());
      this->m_selMeshes.insert(selMeshes.begin(),selMeshes.end());
      // Deprecated start
      for (auto selEntity: selEntities)
      {
      this->m_selEntities.insert(selEntity.entity());
      }
      // Deprecated end
    }

    // \b selection from render window
    else if (modifierFlag == smtk::extension::SelectionModifier::SELECTION_INQUIRY)
    {
      if (this->m_selectionModifier == SelectionModifier::SELECTION_REPLACE_FILTERED)
      { // clear and select
        this->clearAllSelections();
        this->filterEntitySelectionsByMask(const_cast<smtk::model::EntityRefs &>
                                           (selEntities), this->m_selEntityRefs);
        if (this->m_filterMeshes)
          {
          this->m_selMeshes.insert(selMeshes.begin(),selMeshes.end());
          }
        // Deprecated start
        for (auto selEntity: this->m_selEntityRefs)
        {
        this->m_selEntities.insert(selEntity.entity());
        }
        // Deprecated end
      }
      else if (this->m_selectionModifier == SelectionModifier::SELECTION_ADDITION_FILTERED)
      { // add to current selection
        smtk::model::EntityRefs currentSelFiltered;
        this->filterEntitySelectionsByMask(const_cast<smtk::model::EntityRefs &>
                                           (selEntities), currentSelFiltered);
        this->m_selEntityRefs.insert(currentSelFiltered.begin(), currentSelFiltered.end());

        if (this->m_filterMeshes)
          {
          this->m_selMeshes.insert(selMeshes.begin(),selMeshes.end());
          }
        // Deprecated start
        for (auto selEntity: currentSelFiltered)
        {
        this->m_selEntities.insert(selEntity.entity());
        }
        // Deprecated end
      }
      else if (this->m_selectionModifier == SelectionModifier::SELECTION_SUBTRACTION_FILTERED)
      { //subtract from current selection
        smtk::model::EntityRefs currentSelFiltered;
        this->filterEntitySelectionsByMask(const_cast<smtk::model::EntityRefs &>
                                           (selEntities), currentSelFiltered);
        for (const auto& selEnt: currentSelFiltered)
        {
           // Deprecatred start
           this->m_selEntities.erase(selEnt.entity());
           // Deprecatred end
           this->m_selEntityRefs.erase(selEnt);
        }

        if (this->m_filterMeshes)
          {
          for (const auto& selMesh: selMeshes)
            {
            this->m_selMeshes.erase(selMesh);
            }
          }
      }

      this->m_selectionModifier = SelectionModifier::SELECTION_REPLACE_FILTERED; // reset

    }

    // broadcast to rendering view, model tree and attribute panel if they are
    // not in skipList
    // TODO: Since we want to use surface representation for faces, we have to
    // update render view again to use our settings of pqDataRepresentation
    // *true* should be removed
    if (true || std::find(skipList.begin(), skipList.end(), this->m_skipList[0])
                                  == skipList.end())
    {
      emit broadcastToRenderView(this->m_selEntities, smtk::mesh::MeshSets(),
                                 smtk::model::DescriptivePhrases());
    }

    if (std::find(skipList.begin(), skipList.end(), this->m_skipList[1])
                                  == skipList.end())
    {
      emit  broadcastToModelTree(this->m_selEntities,smtk::mesh::MeshSets(),
                               true);
    }

    if (std::find(skipList.begin(), skipList.end(), this->m_skipList[2])
                                  == skipList.end())
    {
      emit broadcastToAttributeView(this->m_selEntities);
    }
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

  void qtSelectionManager::filterMeshes(bool checked)
  {
    this->m_filterMeshes = checked;
  }

  void qtSelectionManager::clearAllSelections()
  {
    this->m_selEntities.clear();
    this->m_selEntityRefs.clear();
    this->m_selMeshes.clear();
    this->m_desPhrases.clear();
  }

  void qtSelectionManager::filterEntitySelectionsByMask(
      smtk::model::EntityRefs &inputEnts, smtk::model::EntityRefs &filteredSelEnts)
  {
    filteredSelEnts.clear();
    // For now rubber band selection only support F/E/V and group
    for(smtk::model::EntityRefs::iterator inputEnt = inputEnts.begin(); inputEnt != inputEnts.end(); inputEnt++)
    {
      smtk::model::EntityRef ent = *inputEnt;
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
