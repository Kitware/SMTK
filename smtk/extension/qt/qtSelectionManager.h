//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME qtSelectionManager - The selection manager for smtk
// .SECTION Description
// .SECTION See Also

#ifndef __smtk__extension_qtSelectionManager_h
#define __smtk__extension_qtSelectionManager_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"
#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/SessionRef.h"
#include "smtk/mesh/MeshSet.h"

namespace smtk
{
  namespace extension
  {

class SMTKQTEXT_EXPORT qtSelectionManager : public QObject
{
  Q_OBJECT
  public:
    qtSelectionManager();
    void getSelectedEntities(smtk::common::UUIDs &selEntities);
    void getSelectedMeshes(smtk::mesh::MeshSets &selMeshes);

  signals:
    // Description
    // Broadcast selection to model tree
    // If you do not block signal, both tree and view would be updated
    void broadcastToModelTree(const smtk::common::UUIDs &
   selEntities, const smtk::mesh::MeshSets &selMeshes, bool blocksignals) const;

    // Description
    // Broadcast selection to render view by entityRef
    void broadcastToRenderView(const smtk::model::EntityRefs&
                selEntities, const smtk::mesh::MeshSets &selMeshes,
                   const smtk::model::DescriptivePhrases &DesPhrases) const;

    // Description
    // Broadcast selection to render view by UUIDs
    void broadcastToRenderView(const smtk::common::UUIDs&
                selEntities, const smtk::mesh::MeshSets &selMeshes,
                   const smtk::model::DescriptivePhrases &DesPhrases) const;
  public slots:

    // Description
    // update selected items from rendering window
    void updateSelectedItems(const smtk::common::UUIDs &
                selEntities, const smtk::mesh::MeshSets &selMeshes);

    // Description
    // update selected items from model tree
    void updateSelectedItems(const smtk::model::EntityRefs&
                selEntities, const smtk::mesh::MeshSets &selMeshes,
                   const smtk::model::DescriptivePhrases &DesPhrases);

    // Description
    // update selected items from attribute
    void updateSelectedItems(const smtk::common::UUIDs & selEntities);

  protected:
    void clearAllSelections();
    smtk::mesh::MeshSets m_selMeshes;
    smtk::common::UUIDs m_selEntities;
    smtk::model::DescriptivePhrases m_desPhrases;

};

  }; // namespace extension

}; // namespace smtk

#endif
