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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"

#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtModelEntityItem.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/Root.h"
#include "smtk/view/Instanced.h"

#include <QPointer>
#include <QDockWidget>
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

// -----------------------------------------------------------------------------

namespace smtk {
  namespace model {

static const std::string pEntityGroupOpName("entity group");

//-----------------------------------------------------------------------------
qtModelView::qtModelView(QWidget* p)
  : QTreeView(p)
{
  QPointer<smtk::model::QEntityItemModel> qmodel = new smtk::model::QEntityItemModel;
  QPointer<smtk::model::QEntityItemDelegate> qdelegate = new smtk::model::QEntityItemDelegate;
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
  QObject::connect(this,
                   SIGNAL(customContextMenuRequested(const QPoint &)),
                   this, SLOT(showContextMenu(const QPoint &)));
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
smtk::model::QEntityItemModel* qtModelView::getModel() const
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
    smtk::model::QEntityItemModel* qmodel = this->getModel();
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
  if(!brOp || !brOp->specification()->isValid())
    {
    std::cout << "No entity group operator to handel the drop!\n";
    return;  
    }

  smtk::model::QEntityItemModel* qmodel = this->getModel();
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
    BitFlags ef = group.membershipMask();
    foreach(QModelIndex sel, this->selectedIndexes())
      {
      // skip entities that are not part of the same model (not supported in discrete session)
      // In the future, when other type sessions also have group ops, we will revisit here.
      smtk::model::Model selmodel =
        this->owningEntityAs<smtk::model::Model>(sel);
      if(selmodel == modelEnt)
        this->recursiveSelect(qmodel->getItem(sel), selentityrefs, ef, true);
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
  this->currentSelectionByMask(selentityrefs,
    CELL_ENTITY | SHELL_ENTITY  | GROUP_ENTITY | MODEL_ENTITY | INSTANCE_ENTITY);

  emit this->entitiesSelected(selentityrefs);
}

//----------------------------------------------------------------------------
void qtModelView::recursiveSelect (smtk::model::DescriptivePhrasePtr dPhrase,
    smtk::model::EntityRefs& selentityrefs, BitFlags entityFlags,
    bool exactMatch)
{
  if(dPhrase)
    {
    BitFlags masked = dPhrase->relatedEntity().entityFlags() & entityFlags;
    if ((masked && entityFlags == ANY_ENTITY) ||
      (!exactMatch && masked) ||
      (exactMatch && masked == entityFlags))
      {
      selentityrefs.insert(dPhrase->relatedEntity());
      }

    smtk::model::DescriptivePhrases sub = dPhrase->subphrases();
    for (smtk::model::DescriptivePhrases::iterator it = sub.begin();
      it != sub.end(); ++it)
      {
      this->recursiveSelect(*it, selentityrefs, entityFlags, exactMatch);
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
    bool searchUp)
{
  smtk::model::QEntityItemModel* qmodel = this->getModel();
  if(!qmodel)
    {
    return;
    }
  foreach(QModelIndex sel, this->selectedIndexes())
    {
    if(searchUp)
      this->owningEntitiesByMask(qmodel->getItem(sel), selentityrefs, entityFlags);      
    else
      this->recursiveSelect(qmodel->getItem(sel), selentityrefs, entityFlags);
    }
}

//----------------------------------------------------------------------------
void qtModelView::selectEntityItems(const smtk::common::UUIDs& selEntities,
                                 bool blocksignal)
{
  smtk::model::QEntityItemModel* qmodel =
    dynamic_cast<smtk::model::QEntityItemModel*>(this->model());

  // Now recursively check which model indices should be selected:
  QItemSelection selItems;
  this->selectionHelper(qmodel, this->rootIndex(), selEntities, selItems);
  this->blockSignals(blocksignal);
  // If we have any items selected, show them
  if(selItems.count())
    {
    this->selectionModel()->select(selItems, QItemSelectionModel::ClearAndSelect);
    this->scrollTo(selItems.value(0).topLeft());
    }
  else 
    this->clearSelection();
  this->blockSignals(false);
}

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

void qtModelView::selectionHelper(
  QEntityItemModel* qmodel,
  const QModelIndex& parentIdx,
  const smtk::common::UUIDs& selEntities,
  QItemSelection& selItems)
{
  // For all the children of this index, see if
  // each child should be selected and then queue its children.
  for (int row=0; row < qmodel->rowCount(parentIdx); ++row)
    {
    QModelIndex idx(qmodel->index(row, 0, parentIdx));
    DescriptivePhrasePtr dPhrase = qmodel->getItem(idx);
    if (dPhrase && selEntities.find(dPhrase->relatedEntityId()) != selEntities.end())
      {
      this->expandToRoot(qmodel, parentIdx);
      QItemSelectionRange sr(idx);
      selItems.append(sr);
      }
    this->selectionHelper(qmodel, idx, selEntities, selItems);
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

void qtModelView::addGroup(BitFlags flag, const std::string& name)
{
  QEntityItemModel* qmodel = this->getModel();
  smtk::model::ManagerPtr pstore = qmodel->manager();
  Models models;
  smtk::model::EntityRef::EntityRefsFromUUIDs(
    models,
    pstore,
    pstore->entitiesMatchingFlags(smtk::model::MODEL_ENTITY));

  if(!models.empty())
    {
    Group bgroup = pstore->addGroup(
      flag, name);
    models.begin()->addGroup(bgroup);
    std::cout << "Added " << bgroup.name() << " to " << models.begin()->name() << "\n";
    }
}

void qtModelView::removeEntityGroup(
  const smtk::model::Model& modelEnt,
  const smtk::model::SessionRef& sessionRef,
  const QList<smtk::model::Group>& groups)
{
  if(groups.count() == 0)
    return;

  OperatorPtr brOp = this->getOp(sessionRef.session(), pEntityGroupOpName);
  if(!brOp || !brOp->specification()->isValid())
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
  if(!brOp || !brOp->specification()->isValid())
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
void qtModelView::showContextMenu(const QPoint &p)
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

  QModelIndex idx = this->indexAt(p);
  smtk::model::SessionRef brSession;

  if ((brSession =
    this->owningEntityAs<smtk::model::SessionRef>(idx)).isValid())
    {
    StringList opNames = brSession.operatorNames();
    std::sort(opNames.begin(), opNames.end()); 
    for(StringList::const_iterator it = opNames.begin();
        it != opNames.end(); ++it)
      {
      QAction* act = this->m_ContextMenu->addAction((*it).c_str());
      QVariant vdata( QString::fromStdString(brSession.entity().toString()) );
      act->setData(vdata);
      QObject::connect(act, SIGNAL(triggered()), this, SLOT(operatorInvoked()));
      }
    this->m_ContextMenu->popup(this->mapToGlobal(p));
    }
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
  smtk::common::UUID sessId( var.toString().toStdString() );

  smtk::model::QEntityItemModel* qmodel = this->getModel();
  smtk::model::SessionPtr session =
    smtk::model::SessionRef(qmodel->manager(), sessId).session();
  if (!session)
    {
    std::cout << "No session available from session: \"" << sessId.toString() << "\"\n";
    return;
    }
  std::string opName = action->text().toStdString();
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
QDockWidget* qtModelView::operatorsDock()
{
  if(this->m_OperatorsDock && this->m_OperatorsWidget)
    {
    return this->m_OperatorsDock;
    }

  qtModelOperationWidget* opWidget = new qtModelOperationWidget();
  QObject::connect(opWidget, SIGNAL(operationRequested(const smtk::model::OperatorPtr&)),
    this, SIGNAL(operationRequested(const smtk::model::OperatorPtr&)));
  QObject::connect(opWidget, SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)),
    this, SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)));
  QObject::connect(opWidget, SIGNAL(modelEntityItemCreated(smtk::attribute::qtModelEntityItem*)),
    this, SIGNAL(modelEntityItemCreated(smtk::attribute::qtModelEntityItem*)));
  QObject::connect(opWidget, SIGNAL(meshSelectionItemCreated(
                   smtk::attribute::qtMeshSelectionItem*,
                   const std::string&, const smtk::common::UUID&)),
    this, SIGNAL(meshSelectionItemCreated(
                 smtk::attribute::qtMeshSelectionItem*,
                 const std::string&, const smtk::common::UUID&)));

  QWidget* dockP = NULL;
  foreach(QWidget *widget, QApplication::topLevelWidgets())
    {
    if(widget->inherits("QMainWindow"))
      {
      dockP = widget;
      break;
      }
    }

  QDockWidget* dw = new QDockWidget(dockP);
  QScrollArea* s = new QScrollArea(dw);
  s->setWidgetResizable(true);
  s->setFrameShape(QFrame::NoFrame);
  s->setObjectName("scrollArea");

  opWidget->setSizePolicy(QSizePolicy::Preferred,
    QSizePolicy::Expanding);
  s->setWidget(opWidget);
  dw->setObjectName("operatorsDockWidget");
  dw->setWidget(s);
  dw->setFloating(true);

  this->m_OperatorsWidget = opWidget;
  this->m_OperatorsDock = dw;
//  this->m_OperatorsDock->hide();
  return dw;
}

//----------------------------------------------------------------------------
void qtModelView::initOperatorsDock(
  const std::string& opName, smtk::model::SessionPtr session)
{
  // make sure the operator widget is created.
  this->operatorsDock()->show();
  SessionRef bs(session->manager(), session->sessionId());

  this->m_OperatorsWidget->setCurrentOperation(opName, session);
  this->m_OperatorsDock->setWindowTitle(bs.flagSummary().c_str());
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

    this->m_OperatorsWidget->setCurrentOperation(brOp);
    this->m_OperatorsDock->setWindowTitle(bs.flagSummary().c_str());
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
  smtk::model::QEntityItemModel* qmodel =
    dynamic_cast<smtk::model::QEntityItemModel*>(this->model());
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
  if(!attrib->isValid())
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
  if(!brOp || !brOp->specification()->isValid())
    return;
  smtk::model::EntityRefs selentityrefs;
  this->recursiveSelect(this->getModel()->getItem(idx), selentityrefs,
    CELL_ENTITY | SHELL_ENTITY  | GROUP_ENTITY |
    MODEL_ENTITY | INSTANCE_ENTITY | SESSION);
  DescriptivePhrasePtr dp = this->getModel()->getItem(idx);
  int vis = dp->relatedEntity().visible() ? 0 : 1;
  if(this->setEntityVisibility(selentityrefs, vis, brOp))
    this->dataChanged(idx, idx);
  this->update();
}

//----------------------------------------------------------------------------
bool qtModelView::setEntityVisibility(
  const smtk::model::EntityRefs& selentityrefs, int vis, OperatorPtr brOp)
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

  std::cout << "set visibility to " << selentityrefs.size() << " entities\n";

  EntityRefs::const_iterator it;
  int numChangingEnts = 0;
  for (it=selentityrefs.begin(); it != selentityrefs.end(); it++)
    {
    numChangingEnts++;
    attrib->associateEntity(*it);
    }
  if(numChangingEnts)
    {
    emit this->operationRequested(brOp);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void qtModelView::changeEntityColor( const QModelIndex& idx)
{
  OperatorPtr brOp = this->getOp(idx, "set property");
  if(!brOp || !brOp->specification()->isValid())
    return;
//  this->recursiveSelect(this->getModel()->getItem(idx), selentityrefs,
//    CELL_ENTITY | SHELL_ENTITY  | GROUP_ENTITY |
//    MODEL_ENTITY | INSTANCE_ENTITY | SESSION);
  DescriptivePhrasePtr dp = this->getModel()->getItem(idx);
  if(dp && dp->relatedEntity().isValid())
    {
    smtk::model::EntityRefs selentityrefs;
    selentityrefs.insert(dp->relatedEntity());

    smtk::model::FloatList rgba(4);
    rgba = dp->relatedColor();
    QColor currentColor = QColor::fromRgbF(rgba[0], rgba[1], rgba[2]);
    QColor newColor = QColorDialog::getColor(currentColor, this,
      "Choose Entity Color", QColorDialog::DontUseNativeDialog);
    if(newColor.isValid() && newColor != currentColor)
      {
      if(this->setEntityColor(selentityrefs, newColor, brOp))
        this->dataChanged(idx, idx);
      }
    }
}

//----------------------------------------------------------------------------
bool qtModelView::setEntityColor(
  const smtk::model::EntityRefs& selentityrefs,
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
/*
  DescriptivePhrasePtr dp = this->getModel()->getItem(idx);
  smtk::model::FloatList rgba(4);
  rgba = dp->relatedColor();
  QColor currentColor = QColor::fromRgbF(rgba[0], rgba[1], rgba[2]);
  QColor newColor = QColorDialog::getColor(currentColor, this);
*/
  if(newColor.isValid())
    {
    colorItem->setNumberOfValues(4);
    colorItem->setValue(0, newColor.redF());
    colorItem->setValue(1, newColor.greenF());
    colorItem->setValue(2, newColor.blueF());
    colorItem->setValue(3, newColor.alphaF());
    }
  else
    colorItem->setNumberOfValues(0);

 //   EntityRefs entities;
 //   EntityRef::EntityRefsFromUUIDs(entities, brOp->manager(), ids);
    std::cout << "set color to " << selentityrefs.size() << " entities\n";

  int numChangingEnts = 0;
  smtk::model::FloatList rgba(4);
  EntityRefs::const_iterator it;
  for (it=selentityrefs.begin(); it != selentityrefs.end(); it++)
    {
    if(newColor.isValid())
      {
      QColor currentColor;
      if((*it).hasColor())
        {
        rgba = (*it).color();
        currentColor = QColor::fromRgbF(rgba[0], rgba[1], rgba[2]);
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
  const QMap<smtk::model::SessionPtr, smtk::common::UUIDs>& brEntities, int vis)
{
  smtk::model::QEntityItemModel* qmodel =
    dynamic_cast<smtk::model::QEntityItemModel*>(this->model());
  foreach(smtk::model::SessionPtr session, brEntities.keys())
    {
    OperatorPtr brOp = this->getOp(session, "set property");
    if(!brOp || !brOp->specification()->isValid())
      continue;
    EntityRefs entities;
    EntityRef::EntityRefsFromUUIDs(entities, qmodel->manager(), brEntities[session]);
    this->setEntityVisibility(entities, vis, brOp);
    }
  // Now recursively check which model indices should be included:
//  QModelIndexList& foundIndexes
  foreach(QModelIndex idx, this->selectedIndexes())
    {
    this->dataChanged(idx, idx);
    }
}

//----------------------------------------------------------------------------
void qtModelView::syncEntityColor(
    const QMap<smtk::model::SessionPtr, smtk::common::UUIDs>& brEntities,
    const QColor& clr)
{
  smtk::model::QEntityItemModel* qmodel =
    dynamic_cast<smtk::model::QEntityItemModel*>(this->model());
  foreach(smtk::model::SessionPtr session, brEntities.keys())
    {
    OperatorPtr brOp = this->getOp(session, "set property");
    if(!brOp || !brOp->specification()->isValid())
      continue;
    EntityRefs entities;
    EntityRef::EntityRefsFromUUIDs(entities, qmodel->manager(), brEntities[session]);
    this->setEntityColor(entities, clr, brOp);
    }
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
  if(!brOp || !brOp->specification()->isValid())
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

  attrib->associateEntity(dp->relatedEntity());
  emit this->operationRequested(brOp);
}
//-----------------------------------------------------------------------------
void qtModelView::updateWithOperatorResult(
    const smtk::model::SessionRef& sref, const OperatorResult& result)
{
  smtk::model::QEntityItemModel* qmodel =
    dynamic_cast<smtk::model::QEntityItemModel*>(this->model());
  QModelIndex top = this->rootIndex();
  for (int row = 0; row < qmodel->rowCount(top); ++row)
    {
    QModelIndex sessIdx = qmodel->index(row, 0, top);
    DescriptivePhrasePtr dp = qmodel->getItem(sessIdx);
    if(dp && (dp->relatedEntity() == sref))
      {
      qmodel->updateWithOperatorResult(sessIdx, result);
      return;
      }
    }
  // this is a new session, mostly from a read operator of a new session
  qmodel->newSessionOperatorResult(sref, result);
}

//-----------------------------------------------------------------------------
void qtModelView::onEntitiesExpunged(
  const smtk::model::EntityRefs& expungedEnts)
{
  if(!this->m_OperatorsWidget)
    return;
  this->m_OperatorsWidget->expungeEntities(expungedEnts);
}

  } // namespace model
} // namespace smtk
