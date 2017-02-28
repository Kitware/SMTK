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
#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/SessionRef.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/model/Session.h"

namespace smtk
{
  namespace extension
  {
  qtSelectionManager::qtSelectionManager()
  {
    this->clearAllSelections();
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
  {
    this->clearAllSelections();
    // update entity and mesh
    this->m_selEntities.insert(selEntities.begin(),selEntities.end());
    this->m_selMeshes.insert(selMeshes.begin(),selMeshes.end());
    // TBD: broacast to attribute panel
    bool blocksignals = true;
    emit  broadcastToModelTree(this->m_selEntities,this->m_selMeshes,
                               blocksignals);
  }

  void qtSelectionManager::updateSelectedItems(const smtk::model::EntityRefs
          &selEntities, const smtk::mesh::MeshSets &selMeshes,
                   const smtk::model::DescriptivePhrases &DesPhrases)

  {
    this->clearAllSelections();
    for (smtk::model::EntityRefs::iterator it = selEntities.begin();
      it != selEntities.end(); ++it)
    {
      this->m_selEntities.insert(it->entity());
    }
    this->m_selMeshes.insert(selMeshes.begin(),selMeshes.end());
    this->m_desPhrases = DesPhrases;
    // TBD: broacast to attribute panel
    emit broadcastToRenderView(selEntities, selMeshes, DesPhrases);

  }

  void qtSelectionManager::updateSelectedItems(const smtk::common::UUIDs
                                               &selEntities)
  {
    this->clearAllSelections();
    this->m_selEntities.insert(selEntities.begin(), selEntities.end());
    // broadcast to model tree and render view
    bool blocksignals = true;
    emit  broadcastToModelTree(this->m_selEntities,smtk::mesh::MeshSets(),
                               blocksignals);
    emit broadcastToRenderView(selEntities, smtk::mesh::MeshSets(),
                               smtk::model::DescriptivePhrases());
  }

  void qtSelectionManager::clearAllSelections()
  {
    this->m_selEntities.clear();
    this->m_selMeshes.clear();
    this->m_desPhrases.clear();
  }



  }; // namespace extension

}; // namespace smtk

