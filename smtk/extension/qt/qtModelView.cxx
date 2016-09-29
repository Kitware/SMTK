//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtModelView.h"

#include "smtk/model/Entity.h"
#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/FloatData.h"
#include "smtk/model/Group.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Model.h"
#include "smtk/model/Manager.h"
#include "smtk/model/StringData.h"

#include "smtk/extension/qt/qtEntityItemDelegate.h"

#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/MeshPhrase.h"
#include "smtk/model/MeshListPhrase.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/MeshItem.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtModelEntityItem.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtOperatorDockWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Collection.h"

#include <QPointer>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMenu>
#include <QAction>
#include <QVariant>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QApplication>
#include <QHeaderView>
#include <QColorDialog>

#include <iomanip>
#include <algorithm>    // std::sort

using namespace smtk::model;
// -----------------------------------------------------------------------------

namespace smtk {
  namespace extension {

static const std::string pEntityGroupOpName("entity group");

//-----------------------------------------------------------------------------
qtModelView::qtModelView(QWidget* p)
  : QTreeView(p)
{
  QPointer<smtk::extension::QEntityItemModel> qmodel = new smtk::extension::QEntityItemModel;
  QPointer<smtk::extension::QEntityItemDelegate> qdelegate = new smtk::extension::QEntityItemDelegate;
  qmodel->setSupportedDragActions(Qt::CopyAction);
  this->setModel(qmodel); // must come after qmodel->setRoot()
  this->setItemDelegate(qdelegate);

  this->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setSortingEnabled(true);

  QSizePolicy expandPolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  this->setSizePolicy(expandPolicy);
  this->setDragDropMode(QAbstractItemView::DragDrop);
  this->setDropIndicatorShown(true);
  this->setDragEnabled(true);
  this->setAcceptDrops(true);
  this->setContextMenuPolicy(Qt::CustomContextMenu);
//  QObject::connect(this,
//                   SIGNAL(customContextMenuRequested(const QPoint &)),
//                   this, SLOT(showContextMenu(const QPoint &)));
  this->m_ContextMenu = NULL;
  this->m_OperatorsDock = NULL;
  this->m_OperatorsWidget = NULL;

  this->header()->setResizeMode(QHeaderView::ResizeToContents);
  QObject::connect(qdelegate,
                   SIGNAL(requestVisibilityChange(const QModelIndex&)),
                   this, SLOT(toggleEntityVisibility(const QModelIndex&)), Qt::QueuedConnection);
  QObject::connect(qdelegate,
                   SIGNAL(requestColorChange(const QModelIndex&)),
                   this, SLOT(changeEntityColor(const QModelIndex&)), Qt::QueuedConnection);
  QObject::connect(qmodel,
                   SIGNAL(phraseTitleChanged(const QModelIndex&)),
                   this, SLOT(changeEntityName(const QModelIndex&)), Qt::QueuedConnection);
  QObject::connect(qmodel,
                   SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                   this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)), Qt::QueuedConnection);
  QObject::connect(qmodel,
                   SIGNAL(newIndexAdded(const QModelIndex&)),
                   this, SLOT(newIndexAdded(const QModelIndex&)), Qt::QueuedConnection);

}

//-----------------------------------------------------------------------------
qtModelView::~qtModelView()
{
  if(this->m_ContextMenu)
    delete this->m_ContextMenu;

//  if(this->m_OperatorsWidget)
//    delete this->m_OperatorsWidget;

  if(this->m_OperatorsDock)
    delete this->m_OperatorsDock;
}

//-----------------------------------------------------------------------------
smtk::extension::QEntityItemModel* qtModelView::getModel() const
{
  return qobject_cast<QEntityItemModel*>(this->model());
}

//-----------------------------------------------------------------------------
void qtModelView::keyPressEvent(QKeyEvent* keyEvent)
{
  this->QTreeView::keyPressEvent(keyEvent);

  // Try to handle Delete key for Entity Group modification
  if (keyEvent && (keyEvent->key() == Qt::Key_Backspace ||
      keyEvent->key() == Qt::Key_Delete))
    {
    smtk::extension::QEntityItemModel* qmodel = this->getModel();
    // Fow now, keep indices that are either groups themselves, or
    // their direct parent is a group
    QMap<smtk::model::Model, QList<smtk::model::Group> >  grpModels;
    QMap<smtk::model::Model, smtk::model::SessionRef >  modelSessions;
    QList<smtk::model::Group> remGroups;
    QMap<smtk::model::Group, smtk::model::EntityRefs> mapGroup2Ents;

    smtk::model::Group grp;
    smtk::model::SessionRef sref;
    smtk::model::Model grpModel;
    foreach(QModelIndex idx, this->selectedIndexes())
      {
      DescriptivePhrasePtr dPhrase = qmodel->getItem(idx);
      if(!dPhrase)
        continue;

      // If the group is already in remGroups, it should be not be in mapGroup2Ents,
      // because the whole group will be removed.
      if((grp = dPhrase->relatedEntity().as<smtk::model::Group>()).isValid())
        {
        // this call will change dPhrase
        sref = this->owningEntityAs<smtk::model::SessionRef>(dPhrase);
        if(this->hasSessionOp(sref, pEntityGroupOpName))
          {
          grpModel = this->owningEntityAs<smtk::model::Model>(idx);
          if(grpModel.isValid())
            {
            mapGroup2Ents.remove(grp);
            remGroups.push_back(grp);
            grpModels[grpModel].push_back(grp);
            modelSessions[grpModel] = sref;
            }
          }
        }
      else if((grp = this->groupParent(dPhrase)).isValid()
              && !remGroups.contains(grp))
        {
        // this call will change dPhrase
        sref = this->owningEntityAs<smtk::model::SessionRef>(dPhrase);
        if(this->hasSessionOp(sref, pEntityGroupOpName))
          {
          grpModel = this->owningEntityAs<smtk::model::Model>(idx);
          if(grpModel.isValid())
            {
            mapGroup2Ents[grp].insert(dPhrase->relatedEntity());
            grpModels[grpModel].push_back(grp);
            modelSessions[grpModel] = sref;
            }
          }
        }
      }

    QMapIterator<smtk::model::Model, QList<smtk::model::Group> > mit(
        grpModels);
    while(mit.hasNext())
      {
      mit.next();
      QListIterator<smtk::model::Group> lit(mit.value());
      QList<smtk::model::Group> remSessGroups;
      while(lit.hasNext())
        {
        smtk::model::Group agrp = lit.next();
        if(remGroups.contains(agrp))
          {
          // we can remove all the groups together with one remove call
          remSessGroups.push_back(agrp);
          remGroups.removeAll(agrp);
          }
        else if(mapGroup2Ents.contains(agrp))
          {
          // we have to do the operation one group at a time.
          this->removeFromEntityGroup(mit.key(), modelSessions[mit.key()],
                                      agrp, mapGroup2Ents[agrp]);
          }
        }
      this->removeEntityGroup(mit.key(), modelSessions[mit.key()],
                              remSessGroups);
      }
    }
}

//-----------------------------------------------------------------------------
void qtModelView::mouseReleaseEvent( QMouseEvent * mouseEvent)
{
  QPoint evtpos = mouseEvent->pos();
  if ( mouseEvent->button() & Qt::RightButton )
    {
    this->showContextMenu(evtpos);
    }

  QTreeView::mouseReleaseEvent( mouseEvent );
}

//-----------------------------------------------------------------------------
void qtModelView::dropEvent(QDropEvent* dEvent)
{
  // The related session has to have a "entity group" operator to process the
  // entities dropped.
  // Currenly, only "discrete" session has this operator, so only
  // drags between entities within the same "discrete" session
  // will be processed.

  // Depends on the QModelIndex we dropped on, the selected
  // entities will be filtered accordingly based on what type of entities
  // the recieving group can take.

  QModelIndex dropIdx = this->indexAt(dEvent->pos());
  if(!this->hasSessionOp(dropIdx, pEntityGroupOpName))
    return;

  OperatorPtr brOp = this->getOp(dropIdx, pEntityGroupOpName);
  if(!brOp || !brOp->specification())
    {
    std::cout << "No entity group operator to handle the drop!\n";
    return;
    }

  smtk::extension::QEntityItemModel* qmodel = this->getModel();
  DescriptivePhrasePtr dp = qmodel->getItem(dropIdx);
  smtk::model::Group group;
  if (dp && (group = dp->relatedEntity().as<smtk::model::Group>()).isValid())
    {
    smtk::model::Model modelEnt =
      this->owningEntityAs<smtk::model::Model>(dropIdx);
    if(!modelEnt.isValid())
      {
      std::cerr << "No owning model found for the entity group!\n";
      return;
      }

/*
    BitFlags ef = (dp->relatedEntity().entityFlags() & DIMENSION_2) ?
      CELL_2D : CELL_3D;
*/
    smtk::model::EntityRefs selentityrefs;
    smtk::model::DescriptivePhrases selproperties;
    BitFlags ef = group.membershipMask();
    foreach(QModelIndex sel, this->selectedIndexes())
      {
      // skip entities that are not part of the same model (not supported in discrete session)
      // In the future, when other type sessions also have group ops, we will revisit here.
      smtk::model::Model selmodel =
        this->owningEntityAs<smtk::model::Model>(sel);
      if(selmodel == modelEnt)
        this->recursiveSelect(qmodel->getItem(sel), selentityrefs, ef, selproperties, true);
      }
    std::cout << selentityrefs.size() << " selentityrefs, " << selentityrefs.size() << " entities\n";
    if(selentityrefs.size() == 0)
      return;

    // prepare the 'entity group' operation
    smtk::attribute::AttributePtr attrib = brOp->specification();
    smtk::attribute::ModelEntityItemPtr modelItem =
      attrib->findModelEntity("model");
    smtk::attribute::StringItem::Ptr optypeItem =
      attrib->findString("Operation");
    smtk::attribute::ModelEntityItemPtr grpItem =
      attrib->findAs<smtk::attribute::ModelEntityItem>(
        "modify cell group", attribute::ALL_CHILDREN);
    smtk::attribute::ModelEntityItemPtr addItem =
      attrib->findAs<smtk::attribute::ModelEntityItem>(
        "cell to add", attribute::ALL_CHILDREN);
    if(!modelItem || !optypeItem || !grpItem || !addItem)
      {
      std::cerr << "The entity group operator's specification is missing items!\n";
      return;
      }

    modelItem->setValue(modelEnt);
    grpItem->setValue(group);
    optypeItem->setValue("Modify");
    if(!addItem->setNumberOfValues(selentityrefs.size()))
      {
      std::cerr << "setNumberOfValues failed for \"cell to add\" item!\n";
      return;
      }
    int i = 0;
    smtk::model::EntityRefs::const_iterator it;
    for(it=selentityrefs.begin(); it!=selentityrefs.end(); ++it)
      addItem->setValue(i++, *it);

//    group.addEntities(selentityrefs);
//    this->getModel()->subphrasesUpdated(dropIdx);
//    this->setExpanded(dropIdx, true);
    if ( dEvent->proposedAction() == Qt::MoveAction )
      {
      //move events break the way we handle drops, convert it to a copy
      dEvent->setDropAction( Qt::CopyAction );
      }
    dEvent->accept();

    emit this->operationRequested(brOp);
    }
}

//-----------------------------------------------------------------------------
Qt::DropActions qtModelView::supportedDropActions () const
{
  // returns what actions are supported when dropping
  return Qt::CopyAction;
}

//-----------------------------------------------------------------------------
void qtModelView::startDrag ( Qt::DropActions supportedActions )
{
//  emit this->dragStarted(this);
  this->QTreeView::startDrag(supportedActions);
}

//-----------------------------------------------------------------------------
void qtModelView::dragEnterEvent ( QDragEnterEvent * eevent )
{
  this->QTreeView::dragEnterEvent(eevent);
}

//-----------------------------------------------------------------------------
void qtModelView::dragMoveEvent( QDragMoveEvent * mevent )
{
  if ( mevent->proposedAction() & this->supportedDropActions() )
    {
    mevent->accept();
    }
}

//-----------------------------------------------------------------------------
void qtModelView::selectionChanged (
    const QItemSelection & selected, const QItemSelection & deselected )
{
  QTreeView::selectionChanged(selected, deselected);
  smtk::model::EntityRefs selentityrefs;
  smtk::model::DescriptivePhrases selproperties;
  smtk::mesh::MeshSets selmeshes;
  this->currentSelectionByMask(selentityrefs,
    CELL_ENTITY | SHELL_ENTITY  | GROUP_ENTITY | MODEL_ENTITY | INSTANCE_ENTITY,
    selproperties, false, &selmeshes);

  emit this->selectionChanged(selentityrefs, selmeshes, selproperties);
}

//-----------------------------------------------------------------------------
// when the dataChanged is emitted from the model, we want to scroll to
// that index so that the changes are visible in the tree view.
void qtModelView::newIndexAdded(const QModelIndex & newidx)
{
  this->scrollTo(newidx);
}

//----------------------------------------------------------------------------
void qtModelView::recursiveSelect (const smtk::model::DescriptivePhrasePtr& dPhrase,
    smtk::model::EntityRefs& selentityrefs, BitFlags entityFlags,
    smtk::model::DescriptivePhrases& selproperties, bool exactMatch,
    smtk::mesh::MeshSets* selmeshes)
{
  if(dPhrase)
    {
    if(selmeshes && (dPhrase->phraseType() == MESH_SUMMARY ||
      dPhrase->phraseType() == MESH_LIST))
      {
      this->selectMeshes(dPhrase, selmeshes);
      }
    else if(dPhrase->phraseType() != MESH_SUMMARY &&
      dPhrase->phraseType() != MESH_LIST)
      {
      BitFlags masked = dPhrase->relatedEntity().entityFlags() & entityFlags;
      if ((masked && entityFlags == ANY_ENTITY) ||
        (!exactMatch && masked) ||
        (exactMatch && masked == entityFlags))
        {
        // if this is property value phrase, add it to the property phrase list
        if (dPhrase->isPropertyValueType())
          {
          selproperties.push_back(dPhrase);
          }
        else // add to the entity list
          {
          selentityrefs.insert(dPhrase->relatedEntity());
          }
        }
      }
    smtk::model::DescriptivePhrases sub = dPhrase->subphrases();
    for (smtk::model::DescriptivePhrases::iterator it = sub.begin();
      it != sub.end(); ++it)
      {
      this->recursiveSelect(*it, selentityrefs, entityFlags, selproperties,
                            exactMatch, selmeshes);
      }
    }
}

//----------------------------------------------------------------------------
void qtModelView::selectMeshes( const DescriptivePhrasePtr& dp,
                               smtk::mesh::MeshSets* selmeshes)
{
  if(!selmeshes)
    {
    return;
    }
  if(dp->phraseType() == MESH_SUMMARY)
    {
    MeshPhrasePtr mphrase = smtk::dynamic_pointer_cast<MeshPhrase>(dp);
    if(mphrase)
      {
      if(mphrase->relatedMeshCollection())
        selmeshes->insert(mphrase->relatedMeshCollection()->meshes());
      else if(!mphrase->relatedMesh().is_empty())
        selmeshes->insert(mphrase->relatedMesh());
      }
    }
}

//----------------------------------------------------------------------------
void qtModelView::owningEntitiesByMask (
    smtk::model::DescriptivePhrasePtr inDp,
    smtk::model::EntityRefs& selentityrefs, BitFlags entityFlags)
{
  DescriptivePhrasePtr dp = inDp;
  while (dp)
    {
    EntityPhrasePtr ephrase = smtk::dynamic_pointer_cast<EntityPhrase>(dp);
    if (ephrase && ephrase->relatedEntity().isValid())
      {
      BitFlags masked = ephrase->relatedEntity().entityFlags() & entityFlags;
      if (entityFlags == ANY_ENTITY || masked)
        {
        selentityrefs.insert(ephrase->relatedEntity());
        break;// stop
        }
      }


    if(!dp->parent()) // could be an EntityListPhrase
      {
      EntityListPhrasePtr lphrase = smtk::dynamic_pointer_cast<EntityListPhrase>(dp);
      if (lphrase)
        dp = lphrase->parent();
      }
    else
      dp = dp->parent();
    }
}

//----------------------------------------------------------------------------
void qtModelView::currentSelectionByMask (
    smtk::model::EntityRefs& selentityrefs, const BitFlags& entityFlags,
    smtk::model::DescriptivePhrases& selproperties,
    bool searchUp, smtk::mesh::MeshSets* selmeshes)
{
  smtk::extension::QEntityItemModel* qmodel = this->getModel();
  if(!qmodel)
    {
    return;
    }
  foreach(QModelIndex sel, this->selectedIndexes())
    {
    if(searchUp)
      this->owningEntitiesByMask(qmodel->getItem(sel),
        selentityrefs, entityFlags);
    else
      this->recursiveSelect(qmodel->getItem(sel),
        selentityrefs, entityFlags, selproperties, false, selmeshes);
    }
}

//----------------------------------------------------------------------------
void qtModelView::selectItems(
  const smtk::common::UUIDs& selEntities,
  const smtk::mesh::MeshSets& selMeshes,
  const std::map<std::string, smtk::common::UUIDs>& property2Entities,
  bool blocksignal)
{
  smtk::extension::QEntityItemModel* qmodel =
    dynamic_cast<smtk::extension::QEntityItemModel*>(this->model());

  // Now recursively check which model indices should be selected:
  QItemSelection selItems;
  this->selectionHelper(qmodel, this->rootIndex(),
                        selEntities, selMeshes, property2Entities, selItems);
  this->blockSignals(blocksignal);
  // If we have any items selected, show them
  if(selItems.count())
    {
    this->selectionModel()->select(selItems, QItemSelectionModel::ClearAndSelect);
    this->scrollTo(selItems.value(0).topLeft());
    }
  else
    {
    this->clearSelection();
    }
  this->blockSignals(false);
}

//----------------------------------------------------------------------------
void qtModelView::selectEntityItems(const smtk::common::UUIDs& selEntities,
                                 bool blocksignal)
{
  this->selectItems(selEntities, smtk::mesh::MeshSets(),
                    std::map<std::string, smtk::common::UUIDs>(), blocksignal);
}

//----------------------------------------------------------------------------
void qtModelView::selectMeshItems(const smtk::mesh::MeshSets& selMeshes,
    bool blocksignal)
{
    this->selectItems(smtk::common::UUIDs(), selMeshes,
                      std::map<std::string, smtk::common::UUIDs>(), blocksignal);
}

//----------------------------------------------------------------------------
void qtModelView::selectPropertyItems(
  const std::map<std::string, smtk::common::UUIDs>& property2Entities,
  bool blocksignal)
{
    this->selectItems(smtk::common::UUIDs(),
                      smtk::mesh::MeshSets(), property2Entities, blocksignal);
}

//----------------------------------------------------------------------------
void qtModelView::expandToRoot(QEntityItemModel* qmodel, const QModelIndex& idx)
{
  if(0)
  {
    std::cout << "idx isValid " << idx.isValid() << std::endl;
    DescriptivePhrasePtr dPhrase = qmodel->getItem(idx);
    if(dPhrase)
    {
      std::cout << "title " << dPhrase->title() << std::endl;
    }
  }
  if(idx.isValid())
    {
    this->setExpanded(idx, true);
    this->expandToRoot(qmodel, qmodel->parent(idx));
    }
}

//----------------------------------------------------------------------------
void qtModelView::selectionHelper(
  QEntityItemModel* qmodel,
  const QModelIndex& parentIdx,
  const smtk::common::UUIDs& selEntities,
  const smtk::mesh::MeshSets& selMeshes,
  const std::map<std::string, smtk::common::UUIDs>& property2Entities,
  QItemSelection& selItems)
{
  // For all the children of this index, see if
  // each child should be selected and then queue its children.
  for (int row=0; row < qmodel->rowCount(parentIdx); ++row)
    {
    bool itemfound = false;
    QModelIndex idx(qmodel->index(row, 0, parentIdx));
    DescriptivePhrasePtr dPhrase = qmodel->getItem(idx);
    if (dPhrase)
      {
      if(dPhrase->isPropertyValueType())
        {
        // if the property is in the selected property list, and the related entity also match,
        // then select the property tree entry
        std::string pName = dPhrase->title();
        std::map<std::string, smtk::common::UUIDs>::const_iterator it =
          property2Entities.find(pName);
        if(it!= property2Entities.end())
          {
          itemfound = it->second.find(dPhrase->relatedEntityId()) != it->second.end();
          }
        }
      else if (selEntities.find(dPhrase->relatedEntityId()) != selEntities.end() ||
        selMeshes.find(dPhrase->relatedMesh()) != selMeshes.end())
        {
        itemfound = true;
        }

      if(itemfound)
        {
        this->expandToRoot(qmodel, parentIdx);
        QItemSelectionRange sr(idx);
        selItems.append(sr);
        }
      this->selectionHelper(qmodel, idx, selEntities, selMeshes,
        property2Entities, selItems);
      }
    }
}

DescriptivePhrasePtr qtModelView::currentItem() const
{
  QModelIndex idx = this->currentIndex();
  if (idx.isValid())
    {
    return this->getModel()->getItem(idx);
    }
  return DescriptivePhrasePtr();
}

void qtModelView::removeEntityGroup(
  const smtk::model::Model& modelEnt,
  const smtk::model::SessionRef& sessionRef,
  const QList<smtk::model::Group>& groups)
{
  if(groups.count() == 0)
    return;

  OperatorPtr brOp = this->getOp(sessionRef.session(), pEntityGroupOpName);
  if(!brOp || !brOp->specification())
    {
    std::cout << "No entity group operator to handle the key press!\n";
    return;
    }

  // prepare the 'entity group' operation
  smtk::attribute::AttributePtr attrib = brOp->specification();
  smtk::attribute::ModelEntityItemPtr modelItem =
    attrib->findModelEntity("model");
  smtk::attribute::StringItem::Ptr optypeItem =
    attrib->findString("Operation");
  smtk::attribute::ModelEntityItemPtr grpItem =
    attrib->findAs<smtk::attribute::ModelEntityItem>(
      "remove cell group", attribute::ALL_CHILDREN);
  if(!modelItem || !optypeItem || !grpItem)
    {
    std::cerr << "The entity group operator's specification is missing items!\n";
    return;
    }

  modelItem->setValue(modelEnt);
  optypeItem->setValue("Remove");
  grpItem->setNumberOfValues(groups.count());
  for (int i = 0; i < groups.count(); ++i)
    {
    grpItem->setValue(i, groups[i]);
    }

  emit this->operationRequested(brOp);
}

void qtModelView::removeFromEntityGroup(
  const smtk::model::Model& modelEnt,
  const smtk::model::SessionRef& sessionRef,
  const smtk::model::Group& grp,
  const smtk::model::EntityRefs& rementities)
{
  if(rementities.size() == 0)
    return;

  OperatorPtr brOp = this->getOp(sessionRef.session(), pEntityGroupOpName);
  if(!brOp || !brOp->specification())
    {
    std::cout << "No entity group operator to handle the key press!\n";
    return;
    }

  // prepare the 'entity group' operation
  smtk::attribute::AttributePtr attrib = brOp->specification();
  smtk::attribute::ModelEntityItemPtr modelItem =
    attrib->findModelEntity("model");
  smtk::attribute::StringItem::Ptr optypeItem =
    attrib->findString("Operation");
  smtk::attribute::ModelEntityItemPtr grpItem =
    attrib->findAs<smtk::attribute::ModelEntityItem>(
      "modify cell group", attribute::ALL_CHILDREN);
  smtk::attribute::ModelEntityItemPtr remItem =
    attrib->findAs<smtk::attribute::ModelEntityItem>(
      "cell to remove", attribute::ALL_CHILDREN);
  if(!modelItem || !optypeItem || !grpItem || !remItem)
    {
    std::cerr << "The entity group operator's specification is missing items!\n";
    return;
    }

  modelItem->setValue(modelEnt);
  grpItem->setValue(grp);
  optypeItem->setValue("Modify");

  if(!remItem->setNumberOfValues(rementities.size()))
    {
    std::cerr << "setNumberOfValues failed for \"cell to remove\" item!\n";
    return;
    }
  int i = 0;
  smtk::model::EntityRefs::const_iterator it;
  for(it=rementities.begin(); it!=rementities.end(); ++it)
    remItem->setValue(i++, *it);

//  grp.removeEntities(rementities);

  emit this->operationRequested(brOp);

}


/**\brief Does \a qidx refer to an entity that is displayed as the child of a group?
  *
  * Note that a group (EntityPhrase with a EntityRef whose isGroup() is true)
  * may contain an EntityListPhrase, each entry of which is in the group.
  * We must test for this 1 level of indirection as well as for direct
  * children.
  */
smtk::model::Group qtModelView::groupParentOfIndex(const QModelIndex& qidx)
{
  smtk::model::Group group;
  DescriptivePhrasePtr phrase = this->getModel()->getItem(qidx);
  return this->groupParent(phrase);
}

smtk::model::Group qtModelView::groupParent(const DescriptivePhrasePtr& phrase)
{
  smtk::model::Group group;
  if (phrase)
    {
    EntityPhrasePtr ephrase = smtk::dynamic_pointer_cast<EntityPhrase>(phrase);
    if (ephrase && ephrase->relatedEntity().isValid())
      {
      DescriptivePhrasePtr pPhrase = ephrase->parent();
      if (pPhrase)
        {
        ephrase = smtk::dynamic_pointer_cast<EntityPhrase>(pPhrase);
        if (ephrase && (group = ephrase->relatedEntity().as<smtk::model::Group>()).isValid())
          return group; // direct child of a Group's summary phrase.
        EntityListPhrasePtr lphrase = smtk::dynamic_pointer_cast<EntityListPhrase>(pPhrase);
        if (lphrase)
          {
          ephrase = smtk::dynamic_pointer_cast<EntityPhrase>(lphrase->parent());
          if (ephrase && (group = ephrase->relatedEntity().as<smtk::model::Group>()).isValid())
            return group; // member of a list inside a Group's summary.
          }
        }
      }
    }
  return group;
}

//-----------------------------------------------------------------------------
void qtModelView::showContextMenu(const QModelIndex &idx, const QPoint& p)
{
  // Set up Context Menu Structure
  if(this->m_ContextMenu)
    {
    this->m_ContextMenu->clear();
    }
  else
    {
    this->m_ContextMenu = new QMenu(this);
    this->m_ContextMenu->setTitle("Operators Menu");
    }

  smtk::model::SessionRef brSession =
    this->owningEntityAs<smtk::model::SessionRef>(idx);

  if (!brSession.isValid())
    {
    // Nothing to do the session is not valid;
    return;
    }
  std::string sessionString = brSession.entity().toString();
  // Have we already processed this session previously?
  if (this->m_sessionInfo.find(sessionString) == this->m_sessionInfo.end())
    {
    // This is the first time seeing this session
    // First we need to get the mapping between operator labels and their names
    std::map<std::string, std::string> opLabelsMap = brSession.session()->operatorLabelsMap(false);
    // Next lets get the list of labels so we can sort them
    std::vector<std::string> keyList;
    for (auto imap: opLabelsMap)
      {
      keyList.push_back(imap.first);
      }
    std::sort(keyList.begin(), keyList.end());
    this->m_sessionInfo[sessionString] =
      std::pair<std::vector<std::string>, std::map<std::string, std::string> >(keyList, opLabelsMap);
    }
  auto sinfo = this->m_sessionInfo[sessionString];
  for(StringList::const_iterator it = sinfo.first.begin();
      it != sinfo.first.end(); ++it)
    {
    QAction* act = this->m_ContextMenu->addAction((*it).c_str());
    QVariant vdata( QString::fromStdString(sessionString) );
    act->setData(vdata);
    QObject::connect(act, SIGNAL(triggered()), this, SLOT(operatorInvoked()));
    }
  QPoint popP = p;
  if(popP.isNull())
    {
    QRect idxRec = this->visualRect(idx);
    popP.setX(idxRec.right()/2);
    popP.setY(idxRec.top());
    }
  this->m_ContextMenu->popup(this->mapToGlobal(popP));
}

//-----------------------------------------------------------------------------
void qtModelView::showContextMenu(const QPoint &p)
{
  QModelIndex idx = this->indexAt(p);
  this->showContextMenu(idx, p);
}

//-----------------------------------------------------------------------------
template<typename T>
T qtModelView::owningEntityAs(const QModelIndex &idx) const
{
  DescriptivePhrasePtr dp = this->getModel()->getItem(idx);
  return this->owningEntityAs<T>(dp);
}

/**\brief Get entity <T> who owns \a dp
  *
  * Like the groupParent() method, a group may contain an EntityListPhrase.
  * We must test for this 1 level of indirection as well as for direct
  * children.
  */
//-----------------------------------------------------------------------------
template<typename T>
T qtModelView::owningEntityAs(const DescriptivePhrasePtr &inDp) const
{
  DescriptivePhrasePtr dp = inDp;
  T entRef;
  while (dp)
    {
    EntityPhrasePtr ephrase = smtk::dynamic_pointer_cast<EntityPhrase>(dp);
    if (ephrase && ephrase->relatedEntity().isValid())
      entRef = ephrase->relatedEntity().as<T>();
    if(entRef.isValid())
      break;

    if(!dp->parent()) // could be an EntityListPhrase
      {
      EntityListPhrasePtr lphrase = smtk::dynamic_pointer_cast<EntityListPhrase>(dp);
      if (lphrase)
        dp = lphrase->parent();
      }
    else
      dp = dp->parent();
    }
  return entRef;
}

//-----------------------------------------------------------------------------
void qtModelView::operatorInvoked()
{
  QAction* const action = qobject_cast<QAction*>(
    QObject::sender());
  if(!action)
    {
    return;
    }
  QVariant var = action->data();
  std::string sessionName = var.toString().toStdString();
  smtk::common::UUID sessId(sessionName);

  smtk::extension::QEntityItemModel* qmodel = this->getModel();
  smtk::model::SessionPtr session =
    smtk::model::SessionRef(qmodel->manager(), sessId).session();
  if (!session)
    {
    std::cout << "No session available from session: \"" << sessId.toString() << "\"\n";
    return;
    }
  std::string opLabel = action->text().toStdString();
  std::string opName  = this->m_sessionInfo[sessionName].second[opLabel];
  this->initOperatorsDock(opName, session);

//  cJSON* json = cJSON_CreateObject();
//  ExportJSON::forOperator(brOp, json);
//  std::cout << "Found operator " << cJSON_Print(json) << ")\n";
//  OperatorResult result = brOp->operate();
//  json = cJSON_CreateObject();
//  ExportJSON::forOperatorResult(result, json);
//  std::cout << "Result " << cJSON_Print(json) << "\n";

//  emit this->operationRequested(uid, action->text());
//  emit this->operationFinished(result);
}

//----------------------------------------------------------------------------
qtOperatorDockWidget* qtModelView::operatorsDock()
{
  if(this->m_OperatorsDock && this->m_OperatorsWidget)
    {
    return this->m_OperatorsDock;
    }

  qtModelOperationWidget* opWidget = new qtModelOperationWidget();
  opWidget->setModelView(this);
  QObject::connect(opWidget, SIGNAL(operationRequested(const smtk::model::OperatorPtr&)),
    this, SIGNAL(operationRequested(const smtk::model::OperatorPtr&)));
  QObject::connect(opWidget, SIGNAL(operationCancelled(const smtk::model::OperatorPtr&)),
    this, SIGNAL(operationCancelled(const smtk::model::OperatorPtr&)));
  QObject::connect(opWidget, SIGNAL(fileItemCreated(smtk::extension::qtFileItem*)),
    this, SIGNAL(fileItemCreated(smtk::extension::qtFileItem*)));
  QObject::connect(opWidget, SIGNAL(modelEntityItemCreated(smtk::extension::qtModelEntityItem*)),
    this, SIGNAL(modelEntityItemCreated(smtk::extension::qtModelEntityItem*)));
  QObject::connect(opWidget, SIGNAL(meshSelectionItemCreated(
                   smtk::extension::qtMeshSelectionItem*,
                   const std::string&, const smtk::common::UUID&)),
    this, SIGNAL(meshSelectionItemCreated(
                 smtk::extension::qtMeshSelectionItem*,
                 const std::string&, const smtk::common::UUID&)));
  QObject::connect(opWidget, SIGNAL(entitiesSelected(const smtk::common::UUIDs&)),
    this, SLOT(selectEntities(const smtk::common::UUIDs&)));

  QWidget* dockP = NULL;
  foreach(QWidget *widget, QApplication::topLevelWidgets())
    {
    if(widget->inherits("QMainWindow"))
      {
      dockP = widget;
      break;
      }
    }

  qtOperatorDockWidget* dw = new qtOperatorDockWidget(dockP);
  QScrollArea* s = new QScrollArea(dw);
  s->setWidgetResizable(true);
  s->setFrameShape(QFrame::NoFrame);
  s->setObjectName("scrollArea");

  opWidget->setSizePolicy(QSizePolicy::Preferred,
    QSizePolicy::Expanding);
  s->setWidget(opWidget);
  dw->setWidget(s);

  QObject::connect(dw, SIGNAL(closing()),
    this, SLOT(onOperationPanelClosing()));

  this->m_OperatorsWidget = opWidget;
  this->m_OperatorsDock = dw;
//  this->m_OperatorsDock->hide();
  return dw;
}

//----------------------------------------------------------------------------
qtModelOperationWidget* qtModelView::operatorsWidget()
{
  if(!this->m_OperatorsWidget)
    {
    this->operatorsDock();
    }
  return this->m_OperatorsWidget;
}

//----------------------------------------------------------------------------
void qtModelView::initOperatorsDock(
  const std::string& opName, smtk::model::SessionPtr session)
{
  // make sure the operator widget is created.
  this->operatorsDock()->show();
  SessionRef bs(session->manager(), session->sessionId());

  this->m_OperatorsWidget->setCurrentOperator(opName, session);
  this->m_OperatorsDock->setWindowTitle(bs.flagSummary().c_str());

  if(QScrollArea* scrollArea = qobject_cast<QScrollArea*>(
    this->m_OperatorsDock->widget()))
    {
    scrollArea->ensureWidgetVisible(this->m_OperatorsWidget);
    }
  // sizeHint() alone doesn't work, so force resize
  this->m_OperatorsDock->resize(this->m_OperatorsWidget->sizeHint());
  this->m_OperatorsDock->updateGeometry();

}

//-----------------------------------------------------------------------------
bool qtModelView::requestOperation(
  const smtk::model::OperatorPtr& brOp, bool launchUI)
{
  if(!brOp)
    {
    return false;
    }

  if(!launchUI)
    {
    emit this->operationRequested(brOp);
    }
  else // launch the m_OperatorsDock
    {
    this->operatorsDock()->show();
    SessionRef bs(brOp->manager(), brOp->session()->sessionId());

    this->m_OperatorsWidget->initOperatorUI(brOp);
    this->m_OperatorsDock->setWindowTitle(bs.flagSummary().c_str());

    if(QScrollArea* scrollArea = qobject_cast<QScrollArea*>(
      this->m_OperatorsDock->widget()))
      {
      scrollArea->ensureWidgetVisible(this->m_OperatorsWidget);
      }
    // sizeHint() alone doesn't work, so force resize
    this->m_OperatorsDock->resize(this->m_OperatorsWidget->sizeHint());
    this->m_OperatorsDock->updateGeometry();
    }
  return true;
//  cJSON* json = cJSON_CreateObject();
//  ExportJSON::forOperator(brOp, json);
//  std::cout << "Found operator " << cJSON_Print(json) << ")\n";
//  OperatorResult result = brOp->operate();
//  json = cJSON_CreateObject();
//  ExportJSON::forOperatorResult(result, json);
//  std::cout << "Result " << cJSON_Print(json) << "\n";

//  emit this->operationRequested(uid, action->text());
//  emit this->operationFinished(result);
}

//-----------------------------------------------------------------------------
bool qtModelView::requestOperation(
    const std::string& opName, const smtk::common::UUID& sessionId, bool launchOp)
{
  smtk::extension::QEntityItemModel* qmodel =
    dynamic_cast<smtk::extension::QEntityItemModel*>(this->model());
  smtk::model::SessionPtr session =
    smtk::model::SessionRef(qmodel->manager(), sessionId).session();

  this->initOperatorsDock(opName, session);
  if(launchOp)
    this->m_OperatorsWidget->onOperate();
  return true;
}

//----------------------------------------------------------------------------
bool qtModelView::hasSessionOp(const QModelIndex& idx,
  const std::string& opname)
{
  smtk::model::SessionRef sref =
    this->owningEntityAs<smtk::model::SessionRef>(idx);
  return this->hasSessionOp(sref, opname);
}

//----------------------------------------------------------------------------
bool qtModelView::hasSessionOp(const smtk::model::SessionRef& brSession,
  const std::string& opname)
{
  if(brSession.isValid())
    {
    StringList opNames = brSession.operatorNames();
    return std::find(opNames.begin(), opNames.end(), opname) != opNames.end();
    }
  return false;
}

//----------------------------------------------------------------------------
OperatorPtr qtModelView::getOp(const QModelIndex& idx,
  const std::string& opname)
{
  smtk::model::SessionRef sref =
    this->owningEntityAs<smtk::model::SessionRef>(idx);
  if(!sref.isValid())
    {
    std::cerr
      << "Could not find session!\n";
    return OperatorPtr();
    }

  if(!this->hasSessionOp(sref, opname))
    {
    std::cout
      << "The requested operator: \"" << opname << "\" for session"
      << " \"" << (sref.session() ? sref.session()->name() : "(invalid)") << "\""
      << " is not part of session operators.\n";
    return OperatorPtr();
    }

  smtk::model::SessionPtr session = sref.session();
  return this->getOp(session, opname);
}

//----------------------------------------------------------------------------
OperatorPtr qtModelView::getOp(const smtk::model::SessionPtr& brSession,
  const std::string& opname)
{
  OperatorPtr brOp = brSession->op(opname);
  if (!brOp)
    {
    std::cerr
      << "Could not create operator: \"" << opname << "\" for session"
      << " \"" << brSession->name() << "\""
      << " (" << brSession->sessionId() << ")\n";
    return OperatorPtr();
    }

  smtk::attribute::AttributePtr attrib = brOp->specification();
  if(!attrib)
    {
    std::cerr
      << "Invalid spec for the op: " << brOp->name() << "\n";
    return OperatorPtr();
    }

  attrib->system()->setRefModelManager(brSession->manager());

  return brOp;
}

//----------------------------------------------------------------------------
void qtModelView::toggleEntityVisibility( const QModelIndex& idx)
{
  OperatorPtr brOp = this->getOp(idx, "set property");
  if(!brOp || !brOp->specification())
    return;

  DescriptivePhrasePtr dp = this->getModel()->getItem(idx);
  smtk::model::EntityRef curRef = dp->relatedEntity();

  // if the DescriptivePhrase is for a property, we only handle model's
  // image_url property to change visibility of the image representation.
  // This is temparary until we have auxiliary geometry for image in smtk model.
  if(dp->phraseType() == FLOAT_PROPERTY_VALUE ||
     dp->phraseType() == INTEGER_PROPERTY_VALUE ||
     (dp->phraseType() == STRING_PROPERTY_VALUE &&
     (!curRef.isModel() || !curRef.hasStringProperty("image_url"))))
    {
    return;
    }

  smtk::model::EntityRefs selentityrefs;
  smtk::mesh::MeshSets selmeshes;
  smtk::model::DescriptivePhrases selproperties;
  this->recursiveSelect(dp, selentityrefs,
    CELL_ENTITY | SHELL_ENTITY  | GROUP_ENTITY |
    MODEL_ENTITY | INSTANCE_ENTITY | SESSION,
    selproperties, false, &selmeshes);
  bool visible = true;
  if(dp->phraseType() == MESH_LIST || dp->phraseType() == MESH_SUMMARY)
    {
    if(selmeshes.size() == 0)
      {
      return; // nothing to do
      }
    smtk::mesh::CollectionPtr c;
    if(dp->phraseType() == MESH_SUMMARY)
      {
      MeshPhrasePtr mphrase = smtk::dynamic_pointer_cast<MeshPhrase>(dp);
      smtk::mesh::MeshSet meshkey;
      if(!mphrase->relatedMesh().is_empty())
        {
        meshkey = mphrase->relatedMesh();
        c = meshkey.collection();
        }
      else
        {
        c = mphrase->relatedMeshCollection();
        meshkey = c->meshes();
        }
      if(c && !meshkey.is_empty() && c->hasIntegerProperty(meshkey, "visible"))
        {
        const IntegerList& prop(c->integerProperty(meshkey, "visible"));
        if(!prop.empty())
          visible = (prop[0] != 0);
        }
      }
    }
  else if(dp->phraseType() == ENTITY_LIST)
    {
    EntityListPhrasePtr lphrase = smtk::dynamic_pointer_cast<EntityListPhrase>(dp);
    // if all its children is off, then show as off
    bool hasVisibleChild = false;
    EntityRefArray::const_iterator it;
    for(it = lphrase->relatedEntities().begin();
        it != lphrase->relatedEntities().end() && !hasVisibleChild; ++it)
      {
      if(it->hasVisibility())
        {
        const IntegerList& prop(it->integerProperty("visible"));
        if(!prop.empty() && prop[0] != 0)
          hasVisibleChild = true;
        }
      else // default is visible
        hasVisibleChild = true;
      }
    visible = hasVisibleChild;
    }
  else if(dp->relatedEntity().hasVisibility())
    {
    const IntegerList& prop(dp->relatedEntity().integerProperty("visible"));
    if(!prop.empty())
      visible = (prop[0] != 0);
    }
  // if only properties are selected, trafer its entity to selentities.
  // This is the case where model's image_url property is selected, we want
  // to pass it back to client for image visibility control.
  if(selentityrefs.size() == 0 && selmeshes.size() == 0 &&
     selproperties.size() > 0)
    {
    for (smtk::model::DescriptivePhrases::const_iterator pit = selproperties.begin();
         pit != selproperties.end(); ++pit)
      {
      curRef = (*pit)->relatedEntity();
      if ((*pit)->phraseType() == STRING_PROPERTY_VALUE &&
          curRef.isValid() && curRef.hasStringProperty("image_url"))
        {
        selentityrefs.insert(curRef);
        }
      }
    }

  int newVis = visible ? 0 : 1;
  if(this->setEntityVisibility(selentityrefs, selmeshes, newVis, brOp))
    this->dataChanged(idx, idx);
  this->update();
}

//----------------------------------------------------------------------------
bool qtModelView::setEntityVisibility(
  const smtk::model::EntityRefs& selentityrefs,
  const smtk::mesh::MeshSets& selmeshes,
  int vis, OperatorPtr brOp)
{
  smtk::attribute::AttributePtr attrib = brOp->specification();
  smtk::attribute::StringItemPtr nameItem =
    attrib->findString("name");
  smtk::attribute::IntItemPtr visItem =
    attrib->findInt("integer value");
  if(!nameItem || !visItem)
    {
    std::cerr
      << "The set-property op is missing item(s): name or integer value\n";
    return false;
    }
  nameItem->setNumberOfValues(1);
  nameItem->setValue("visible");
  visItem->setNumberOfValues(1);
  visItem->setValue(vis);

  EntityRefs::const_iterator it;
  int numChangingEnts = 0;
  // for model entities
  for (it=selentityrefs.begin(); it != selentityrefs.end(); it++)
    {
    numChangingEnts++;
    attrib->associateEntity(*it);
    }
  // for meshes
  if(selmeshes.size())
    {
    smtk::attribute::MeshItemPtr meshItem =
      attrib->findMesh("meshes");
    if(meshItem)
      {
      meshItem->appendValues(selmeshes);
      numChangingEnts += selmeshes.size();
      }
    }

  if(numChangingEnts)
    {
    emit this->operationRequested(brOp);
    return true;
    }
  return false;
}

QColor internal_convertColor(const FloatList& rgba)
{
  int ncomp = static_cast<int>(rgba.size());
  float alpha = ncomp != 4 ? 1. : std::max(0., std::min(rgba[3], 1.0));
  return ncomp >= 3 ?
    QColor::fromRgbF(rgba[0], rgba[1], rgba[2], alpha) : QColor(); 
}

//----------------------------------------------------------------------------
void qtModelView::changeEntityColor( const QModelIndex& idx)
{
  OperatorPtr brOp = this->getOp(idx, "set property");
  if(!brOp || !brOp->specification())
    return;

  DescriptivePhrasePtr dp = this->getModel()->getItem(idx);
  if(!dp)
    return;

  smtk::model::EntityRefs selentityrefs;
  smtk::mesh::MeshSets selmeshes;
  QColor currentColor = Qt::white;
  if(dp->phraseType() == MESH_SUMMARY)
    {
    MeshPhrasePtr mphrase = smtk::dynamic_pointer_cast<MeshPhrase>(dp);
    smtk::mesh::CollectionPtr c;
    smtk::mesh::MeshSet meshkey;
    if(!mphrase->relatedMesh().is_empty())
      {
      meshkey = mphrase->relatedMesh();
      c = meshkey.collection();
      }
    else
      {
      c = mphrase->relatedMeshCollection();
      meshkey = c->meshes();
      }

    if(c && !meshkey.is_empty())
      {
      const FloatList& rgba(c->floatProperty(meshkey, "color"));
      currentColor = internal_convertColor(rgba);
      selmeshes.insert(meshkey);
      }
    }
  else if(!dp->isPropertyValueType() && dp->relatedEntity().isValid())
    {
    selentityrefs.insert(dp->relatedEntity());

    smtk::model::FloatList rgba(4);
    rgba = dp->relatedEntity().color();
    currentColor = internal_convertColor(rgba);
    }

  if(selentityrefs.size() > 0 || selmeshes.size() > 0)
    {
    QColor newColor = QColorDialog::getColor(currentColor, this,
      "Choose Entity Color",
      QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);
    if(newColor.isValid() && newColor != currentColor)
      {
      if(this->setEntityColor(selentityrefs, selmeshes, newColor, brOp))
        this->dataChanged(idx, idx);
      }
    }
}

//----------------------------------------------------------------------------
bool qtModelView::setEntityColor(
  const smtk::model::EntityRefs& selentityrefs,
  const smtk::mesh::MeshSets& selmeshes,
  const QColor& newColor, OperatorPtr brOp)
{
  smtk::attribute::AttributePtr attrib = brOp->specification();
  smtk::attribute::StringItemPtr nameItem =
    attrib->findString("name");
  smtk::attribute::DoubleItemPtr colorItem =
    attrib->findDouble("float value");
  if(!nameItem || !colorItem)
    {
    std::cerr
      << "The set-property op is missing item(s): name or integer value\n";
    return false;
    }
  nameItem->setNumberOfValues(1);
  nameItem->setValue("color");
  if(newColor.isValid())
    {
    colorItem->setNumberOfValues(4);
    colorItem->setValue(0, newColor.redF());
    colorItem->setValue(1, newColor.greenF());
    colorItem->setValue(2, newColor.blueF());
    colorItem->setValue(3, newColor.alphaF());
    }
  else
    {
    colorItem->setNumberOfValues(4);
    std::vector<double> nullColor(4, -1.);
    colorItem->setValues(nullColor.begin(), nullColor.end());
    }

  int numChangingEnts = 0;
  smtk::model::FloatList rgba(4);
  // for model entities
  EntityRefs::const_iterator it;
  for (it=selentityrefs.begin(); it != selentityrefs.end(); it++)
    {
    if(newColor.isValid())
      {
      QColor currentColor = Qt::white;
      if((*it).hasColor())
        {
        rgba = (*it).color();
        currentColor = internal_convertColor(rgba);
        }
      if(newColor != currentColor)
        {
        numChangingEnts++;
        attrib->associateEntity(*it);
        }
      }
    else if((*it).hasColor()) // remove "color" property
      {
      numChangingEnts++;
      attrib->associateEntity(*it);
      }
    }

  // for meshes
  if(selmeshes.size())
    {
    smtk::attribute::MeshItemPtr meshItem =
      attrib->findMesh("meshes");
    if(meshItem)
      {
      meshItem->appendValues(selmeshes);
      numChangingEnts += selmeshes.size();
      }
    }

  if(numChangingEnts)
    {
    emit this->operationRequested(brOp);
    return true;
    }

  return false;
}
//----------------------------------------------------------------------------
/*
void qtModelView::findIndexes(
  QEntityItemModel* qmodel,
  const QModelIndex& parentIdx,
  const smtk::common::UUIDs& selEntities,
  QModelIndexList& foundIndexes)
{
  // For all the children of this index, see if
  // each child should be included and then queue its children.
  for (int row=0; row < qmodel->rowCount(parentIdx); ++row)
    {
    QModelIndex idx(qmodel->index(row, 0, parentIdx));
    DescriptivePhrasePtr dPhrase = qmodel->getItem(idx);
    if (dPhrase && selEntities.find(dPhrase->relatedEntityId()) != selEntities.end())
      {
      foundIndexes.append(idx);
      }
    this->findIndexes(qmodel, idx, selEntities, foundIndexes);
    }
}
*/
//----------------------------------------------------------------------------
void qtModelView::syncEntityVisibility(
                                      const smtk::model::SessionPtr& session,
                                      const smtk::common::UUIDs& entuids,
                                      const smtk::mesh::MeshSets& meshes,
                                      int vis)
{
  if(!session)
    {
    std::cerr << "Input session in null, no op can be created\n";
    return;
    }
  OperatorPtr brOp = this->getOp(session, "set property");
  if(!brOp || !brOp->specification())
    return;
  EntityRefs entities;
  smtk::extension::QEntityItemModel* qmodel =
    dynamic_cast<smtk::extension::QEntityItemModel*>(this->model());
  EntityRef::EntityRefsFromUUIDs(entities, qmodel->manager(), entuids);
  this->setEntityVisibility(entities, meshes, vis, brOp);

  // signal qmodel index data changed
  foreach(QModelIndex idx, this->selectedIndexes())
    {
    this->dataChanged(idx, idx);
    }
}

//----------------------------------------------------------------------------
void qtModelView::syncEntityColor(
                                  const smtk::model::SessionPtr& session,
                                  const smtk::common::UUIDs& entuids,
                                  const smtk::mesh::MeshSets& meshes,
                                  const QColor& clr)
{
  if(!session)
    {
    std::cerr << "Input session in null, no op can be created\n";
    return;
    }
  OperatorPtr brOp = this->getOp(session, "set property");
  if(!brOp || !brOp->specification())
    return;
  EntityRefs entities;
  smtk::extension::QEntityItemModel* qmodel =
    dynamic_cast<smtk::extension::QEntityItemModel*>(this->model());
  EntityRef::EntityRefsFromUUIDs(entities, qmodel->manager(), entuids);
  this->setEntityColor(entities, meshes, clr, brOp);

  // update index to redraw
  foreach(QModelIndex idx, this->selectedIndexes())
    {
    this->dataChanged(idx, idx);
    }
}

//----------------------------------------------------------------------------
void qtModelView::changeEntityName( const QModelIndex& idx)
{
  OperatorPtr brOp = this->getOp(idx, "set property");
  if(!brOp || !brOp->specification())
    return;
  DescriptivePhrasePtr dp = this->getModel()->getItem(idx);
  smtk::attribute::AttributePtr attrib = brOp->specification();
  smtk::attribute::StringItemPtr nameItem =
    attrib->findString("name");
  smtk::attribute::StringItemPtr titleItem =
    attrib->findString("string value");
  if(!nameItem || !titleItem)
    {
    std::cerr
      << "The set-property op is missing item(s): name or string value\n";
    return;
    }
  // change the entity "name" property to its descriptive phrase's new title
  nameItem->setNumberOfValues(1);
  nameItem->setValue("name");
  titleItem->setNumberOfValues(1);
  titleItem->setValue(dp->title());

  if(dp->phraseType() == MESH_SUMMARY)
    {
    MeshPhrasePtr mphrase = smtk::dynamic_pointer_cast<MeshPhrase>(dp);
    smtk::mesh::MeshSet meshkey;
    if(!mphrase->relatedMesh().is_empty())
      {
      meshkey = mphrase->relatedMesh();
      }
    else
      {
      smtk::mesh::ManagerPtr meshMgr = brOp->manager()->meshes();
      smtk::mesh::CollectionPtr c = mphrase->relatedMeshCollection();
      meshkey = c->meshes();
      }
    smtk::attribute::MeshItemPtr meshItem =
      attrib->findMesh("meshes");
    if(meshItem && !meshkey.is_empty())
      {
      meshItem->appendValue(meshkey);
      emit this->operationRequested(brOp);
      }
    }
  else if(dp->relatedEntity().isValid())
    {
    attrib->associateEntity(dp->relatedEntity());
    emit this->operationRequested(brOp);
    }
}
//-----------------------------------------------------------------------------
void qtModelView::updateWithOperatorResult(
    const smtk::model::SessionRef& sref, const OperatorResult& result)
{
  smtk::extension::QEntityItemModel* qmodel =
    dynamic_cast<smtk::extension::QEntityItemModel*>(this->model());
  QModelIndex top = this->rootIndex();
  for (int row = 0; row < qmodel->rowCount(top); ++row)
    {
    QModelIndex sessIdx = qmodel->index(row, 0, top);
    DescriptivePhrasePtr dp = qmodel->getItem(sessIdx);
    if(dp && (dp->relatedEntity() == sref))
      {
      qmodel->updateWithOperatorResult(sessIdx, result);
      this->setExpanded(sessIdx, true);

      // find and expand the new model
      smtk::attribute::ModelEntityItem::Ptr newEntities =
        result->findModelEntity("created");
      if(!newEntities)
        return;
      for (int mrow=0; mrow < qmodel->rowCount(sessIdx); ++mrow)
        {
        QModelIndex modelIdx(qmodel->index(mrow, 0, sessIdx));
        DescriptivePhrasePtr modp = qmodel->getItem(modelIdx);
        // all phrases should be for models in the session
        if(modp && newEntities->has(modp->relatedEntity()))
          {
          this->setExpanded(modelIdx, true);
          }
        }
      return;
      }
    }
  // this is a new session, mostly from a read operator of a new session
  qmodel->newSessionOperatorResult(sref, result);
  // expand the models inside the new session
  for (int row = 0; row < qmodel->rowCount(top); ++row)
    {
    QModelIndex sessIdx = qmodel->index(row, 0, top);
    DescriptivePhrasePtr dp = qmodel->getItem(sessIdx);
    if(dp && (dp->relatedEntity() == sref))
      {
      this->setExpanded(sessIdx, true);
      for (int mrow=0; mrow < qmodel->rowCount(sessIdx); ++mrow)
        {
        QModelIndex modelIdx(qmodel->index(mrow, 0, sessIdx));
        this->setExpanded(modelIdx, true);
        }
      break;
      }
    }
}

//-----------------------------------------------------------------------------
void qtModelView::onEntitiesExpunged(
  const smtk::model::EntityRefs& expungedEnts)
{
  if(!this->m_OperatorsWidget)
    return;
  this->m_OperatorsWidget->expungeEntities(expungedEnts);
}

//-----------------------------------------------------------------------------
std::string qtModelView::determineAction (const QPoint& evtpos) const
{
  QModelIndex idx = this->indexAt(evtpos);
  QEntityItemDelegate* thedelegate = qobject_cast<QEntityItemDelegate*>(this->itemDelegate());
  if(idx.isValid() && thedelegate)
    {
    QStyleOptionViewItem opt = this->viewOptions();
    opt.rect = this->visualRect(idx);

    return thedelegate->determineAction(evtpos, idx, opt);
    }
  return "";
}

//-----------------------------------------------------------------------------
void qtModelView::onOperationPanelClosing()
{
  // If the operation panel is closing, cancel current operation
  if(this->m_OperatorsWidget)
    {
    this->m_OperatorsWidget->cancelCurrentOperator();
    }
}

//-----------------------------------------------------------------------------
void qtModelView::syncEntityVisibility(
    const smtk::common::UUID& sessid,
    const smtk::common::UUIDs& entids,
    const smtk::mesh::MeshSets& meshes,
    int vis)
{
  smtk::extension::QEntityItemModel* qmodel =
    dynamic_cast<smtk::extension::QEntityItemModel*>(this->model());
  smtk::model::SessionPtr session =
    smtk::model::SessionRef(qmodel->manager(), sessid).session();
  this->syncEntityVisibility(session, entids, meshes, vis);
}

//-----------------------------------------------------------------------------
void qtModelView::syncEntityColor(
    const smtk::common::UUID& sessid,
    const smtk::common::UUIDs& entids,
    const smtk::mesh::MeshSets& meshes,
    const QColor& clr)
{
  smtk::extension::QEntityItemModel* qmodel =
    dynamic_cast<smtk::extension::QEntityItemModel*>(this->model());
  smtk::model::SessionPtr session =
    smtk::model::SessionRef(qmodel->manager(), sessid).session();
  this->syncEntityColor(session, entids, meshes, clr);
}

//-----------------------------------------------------------------------------
void qtModelView::expandAllModels()
{
  smtk::extension::QEntityItemModel* qmodel = this->getModel();
  QModelIndex top = this->rootIndex();
  for (int row = 0; row < qmodel->rowCount(top); ++row)
    {
    QModelIndex sessIdx = qmodel->index(row, 0, top);
    this->setExpanded(sessIdx, true);
    for (int mrow=0; mrow < qmodel->rowCount(sessIdx); ++mrow)
      {
      QModelIndex modelIdx(qmodel->index(mrow, 0, sessIdx));
      this->setExpanded(modelIdx, true);
      }
    }
}

  } // namespace model
} // namespace smtk
