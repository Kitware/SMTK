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
#include "smtk/model/GroupEntity.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/StringData.h"

#include "smtk/extension/qt/qtEntityItemDelegate.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/BridgeSession.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/view/Root.h"
#include "smtk/view/Instanced.h"

#include <QPointer>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMenu>
#include <QAction>
#include <QVariant>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include <iomanip>
// -----------------------------------------------------------------------------

namespace smtk {
  namespace model {


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
}

//-----------------------------------------------------------------------------
qtModelView::~qtModelView()
{
}

//-----------------------------------------------------------------------------
smtk::model::QEntityItemModel* qtModelView::getModel() const
{
  return qobject_cast<QEntityItemModel*>(this->model());
}

//-----------------------------------------------------------------------------
void qtModelView::dropEvent(QDropEvent* dEvent)
{
  smtk::model::QEntityItemModel* qmodel = this->getModel();
  smtk::common::UUIDs ids;

  // depends on the QModelIndex we dropped on, the selected
  // entities will be filtered accordingly based on what type of entities
  // the recieving group can take

  QModelIndex dropIdx = this->indexAt(dEvent->pos());
  DescriptivePhrasePtr dp = this->getModel()->getItem(dropIdx);
  smtk::model::GroupEntity group;
  if (dp && (group = dp->relatedEntity().as<smtk::model::GroupEntity>()).isValid())
//  if(dp && dp->relatedEntity().isGroupEntity() )
    {
    BitFlags ef = (dp->relatedEntity().entityFlags() & DIMENSION_2) ?
      CELL_2D : CELL_3D;
    foreach(QModelIndex sel, this->selectedIndexes())
      {
  //    DescriptivePhrasePtr childp = this->getModel()->getItem(sel);
  //    group.addEntity(childp->relatedEntity());
      this->recursiveSelect(qmodel, sel, ids, ef);
      }
    Cursors entities;
    Cursor::CursorsFromUUIDs(entities, qmodel->manager(), ids);
    std::cout << ids.size() << " ids, " << entities.size() << " entities\n";

    group.addEntities(entities);
    this->getModel()->subphrasesUpdated(dropIdx);
    this->setExpanded(dropIdx, true);
    if ( dEvent->proposedAction() == Qt::MoveAction )
      {
      //move events break the way we handle drops, convert it to a copy
      dEvent->setDropAction( Qt::CopyAction );
      }
    dEvent->accept();
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
  smtk::model::QEntityItemModel* qmodel = this->getModel();
  if(!qmodel)
    {
    return;
    }
  QTreeView::selectionChanged(selected, deselected);
  smtk::common::UUIDs ids;
  foreach(QModelIndex sel, this->selectedIndexes())
    {
    this->recursiveSelect(qmodel, sel, ids, CELL_ENTITY);
    }

  emit this->entitiesSelected(ids);
}

//----------------------------------------------------------------------------
void qtModelView::recursiveSelect (
   smtk::model::QEntityItemModel* qmodel, const QModelIndex& sel,
    smtk::common::UUIDs& ids, BitFlags entityFlags)
{
  DescriptivePhrasePtr dPhrase = qmodel->getItem(sel);
  if(dPhrase && (dPhrase->relatedEntity().entityFlags() & entityFlags) == entityFlags &&
  ids.find(dPhrase->relatedEntityId()) == ids.end())
    ids.insert(dPhrase->relatedEntityId());

  for (int row=0; row < qmodel->rowCount(sel); ++row)
    {
    this->recursiveSelect(qmodel, qmodel->index(row, 0, sel), ids, entityFlags);
    }
}

//----------------------------------------------------------------------------
void qtModelView::selectEntities(const QList<std::string>& selIds)
{
  smtk::model::QEntityItemModel* qmodel =
    dynamic_cast<smtk::model::QEntityItemModel*>(this->model());
  // Convert selection to UUIDs for faster membership checks:
  smtk::common::UUIDs selEntities;
  foreach(std::string strId, selIds)
    {
    smtk::common::UUID uid = smtk::common::UUID(strId);
    if (!uid.isNull())
      selEntities.insert(uid);
    }
  // Now recursively check which model indices should be selected:
  QItemSelection selItems;
  this->selectionHelper(qmodel, this->rootIndex(), selEntities, selItems);
  // If we have any items selected, show them
  if(selItems.count())
    {
    this->blockSignals(true);
    this->selectionModel()->select(selItems, QItemSelectionModel::ClearAndSelect);
    this->blockSignals(false);
    this->scrollTo(selItems.value(0).topLeft());
    }
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
  ModelEntities models;
  smtk::model::Cursor::CursorsFromUUIDs(
    models,
    pstore,
    pstore->entitiesMatchingFlags(smtk::model::MODEL_ENTITY));

  if(!models.empty())
    {
    GroupEntity bgroup = pstore->addGroup(
      flag, name);
    models.begin()->addGroup(bgroup);
    std::cout << "Added " << bgroup.name() << " to " << models.begin()->name() << "\n";
    }
}

void qtModelView::removeSelected()
{
  QEntityItemModel* qmodel = this->getModel();
  smtk::model::ManagerPtr pstore = qmodel->manager();
  ModelEntities models;
  smtk::model::Cursor::CursorsFromUUIDs(
    models,
    pstore,
    pstore->entitiesMatchingFlags(smtk::model::MODEL_ENTITY));
  if(models.empty())
    {
    return;
    }

  // remove the groups later, in case some of the group's
  // children may also be selected.
  QList<smtk::model::GroupEntity> groups;
  foreach(QModelIndex sel, this->selectedIndexes())
    {
    DescriptivePhrasePtr dp = this->getModel()->getItem(sel);
    smtk::model::GroupEntity group;
    if(dp && (group = dp->relatedEntity().as<smtk::model::GroupEntity>()).isValid())
      {
      groups.append(group);
      }
    else
      {
      this->removeFromGroup(sel);
      }
    }

  foreach(smtk::model::GroupEntity group, groups)
    {
    models.begin()->removeGroup(group);
    pstore->erase(group.entity());
    }
}

void qtModelView::removeFromGroup(const QModelIndex& qidx)
{
  smtk::model::GroupEntity group;
  if ((group = this->groupParentOfIndex(qidx)).isValid())
    {
    DescriptivePhrasePtr phrase = this->getModel()->getItem(qidx);
    if (phrase)
      {
      // Removing from the group emits a signal that
      // m_p->qmodel listens for, causing m_p->modelTree redraw.
      group.removeEntity(phrase->relatedEntity());
      }
    }
}

/**\brief Does \a qidx refer to an entity that is displayed as the child of a group?
  *
  * Note that a group (EntityPhrase with a Cursor whose isGroup() is true)
  * may contain an EntityListPhrase, each entry of which is in the group.
  * We must test for this 1 level of indirection as well as for direct
  * children.
  */
smtk::model::GroupEntity qtModelView::groupParentOfIndex(const QModelIndex& qidx)
{
  smtk::model::GroupEntity group;
  DescriptivePhrasePtr phrase = this->getModel()->getItem(qidx);
  if (phrase)
    {
    EntityPhrasePtr ephrase = smtk::dynamic_pointer_cast<EntityPhrase>(phrase);
    if (ephrase && ephrase->relatedEntity().isValid())
      {
      phrase = ephrase->parent();
      if (phrase)
        {
        ephrase = smtk::dynamic_pointer_cast<EntityPhrase>(phrase);
        if (ephrase && (group = ephrase->relatedEntity().as<smtk::model::GroupEntity>()).isValid())
          return group; // direct child of a GroupEntity's summary phrase.
        EntityListPhrasePtr lphrase = smtk::dynamic_pointer_cast<EntityListPhrase>(phrase);
        if (lphrase)
          {
          ephrase = smtk::dynamic_pointer_cast<EntityPhrase>(lphrase->parent());
          if (ephrase && (group = ephrase->relatedEntity().as<smtk::model::GroupEntity>()).isValid())
            return group; // member of a list inside a GroupEntity's summary.
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

  QModelIndex dropIdx = this->indexAt(p);
  DescriptivePhrasePtr dp = this->getModel()->getItem(dropIdx);
  smtk::model::BridgeSession brSession;

  if (dp && (brSession = dp->relatedEntity().as<smtk::model::BridgeSession>()).isValid())
    {
    StringList opNames = brSession.operatorNames();
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
  smtk::model::BridgePtr bridge =
    qmodel->manager()->findBridgeSession(sessId);
  if (!bridge)
    {
    std::cout << "No bridge available from bridgeSession: \"" << sessId.toString() << "\"\n";
    return;
    }
  std::string opName = action->text().toStdString();
  OperatorPtr brOp = bridge->op(opName, qmodel->manager());
  if (!brOp)
    {
    std::cerr
      << "Could not create operator: \"" << opName << "\" for bridge"
      << " \"" << bridge->name() << "\""
      << " (" << bridge->sessionId() << ")\n";
    return;
    }

  if(!this->initOperator(brOp))
    return;
  emit this->operationRequested(brOp);

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
bool qtModelView::initOperator(smtk::model::OperatorPtr op)
{
  if(!op ||!op->ensureSpecification() || !op->specification()->isValid())
    {
    return false;
    }
  smtk::attribute::AttributePtr att = op->specification();
  attribute::DefinitionPtr attDef = att->definition();

  QDialog attDialog;
  attDialog.setWindowTitle(attDef->label().empty() ?
                            attDef->type().c_str() : attDef->label().c_str());
  QVBoxLayout* layout = new QVBoxLayout(&attDialog);

  smtk::attribute::qtUIManager uiManager(*(att->system()));
  smtk::view::RootPtr rootView = uiManager.attSystem()->rootView();
  smtk::view::InstancedPtr instanced = smtk::view::Instanced::New(op->name());
  instanced->addInstance(att);
  rootView->addSubView(instanced);
  QObject::connect(&uiManager, SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)),
    this, SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)));

  attribute::qtBaseView* view = uiManager.initializeView(&attDialog, instanced, false);

//  attribute::qtAttribute attWidget(att, &attDialog, NULL);
  //layout->addWidget(attView.widget())
  QDialogButtonBox* buttonBox=new QDialogButtonBox( &attDialog );
  buttonBox->setStandardButtons(QDialogButtonBox::Ok);
  layout->addWidget(buttonBox);
  attDialog.setModal(true);
  QObject::connect(buttonBox, SIGNAL(accepted()), &attDialog, SLOT(accept()));
  return attDialog.exec() == QDialog::Accepted;
}

  } // namespace model
} // namespace smtk
