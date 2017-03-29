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
#include "smtk/model/EntityTypeBits.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/SessionRef.h"

namespace smtk
{
  namespace extension
  {

class SMTKQTEXT_EXPORT qtSelectionManager : public QObject
{
public:
  enum class SelectionModifier
  {
    SELECTION_DEFAULT = 0,
    SELECTION_ADDITION = 1,
    SELECTION_SUBTRACTION = 2

  };

  Q_OBJECT
  public:
    qtSelectionManager();
    void setModelManager(smtk::model::ManagerPtr mgrPtr)
    {this->m_modelMgr = mgrPtr;}
    void getSelectedEntities(smtk::common::UUIDs &selEntities);
    void getSelectedMeshes(smtk::mesh::MeshSets &selMeshes);

    void setSelectionModifierToAddition()
      {this->m_selectionModifier = SelectionModifier::SELECTION_ADDITION;}

    void setSelectionModifierToSubtraction()
      {this->m_selectionModifier = SelectionModifier::SELECTION_SUBTRACTION;}

  signals:
    // Description
    // Broadcast selection to model tree
    // If you do not block signal, both tree and view would be updated
    // It's preferred to set it to true and emit a seperate signal to update
    // render view
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
    // Description
    // Broadcast selection to attrite panel
    void broadcastToAttributeView(const smtk::common::UUIDs &
   selEntities) const;
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


    // Description
    // update mask for model
    void filterModels(bool checked);

    // Description
    // update mask for model
    void filterVolumes(bool checked);

    // Description
    // update mask for model
    void filterFaces(bool checked);

    // Description
    // update mask for model
    void filterEdges(bool checked);

    // Description
    // update mask for model
    void filterVertices(bool checked);

  protected:
    void clearAllSelections();

    // Description
    // filter select entitiesa from currentSelEnt and store the result in
    // filteredSelEnt
    void filterEntitySelectionsByMask(
      smtk::common::UUIDs &currentSelEnt, smtk::common::UUIDs &filteredSelEnt);
    // Description
    // filter selection to handle mask other than F/E/V
    void filterRubberBandSelection(smtk::model::EntityRef ent);

    smtk::mesh::MeshSets m_selMeshes;
    smtk::common::UUIDs m_selEntities;
    smtk::model::DescriptivePhrases m_desPhrases;
    smtk::model::BitFlags m_mask;
    smtk::model::ManagerPtr m_modelMgr;
    SelectionModifier m_selectionModifier;

};

  }; // namespace extension

}; // namespace smtk

#endif
