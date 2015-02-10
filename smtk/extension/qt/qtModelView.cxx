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
#include "smtk/model/SessionRef.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/extension/qt/qtAttribute.h"
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
void qtModelView::dropEvent(QDropEvent* dEvent)
{
  smtk::model::QEntityItemModel* qmodel = this->getModel();
  smtk::model::EntityRefs selentityrefs;

  // depends on the QModelIndex we dropped on, the selected
  // entities will be filtered accordingly based on what type of entities
  // the recieving group can take

  QModelIndex dropIdx = this->indexAt(dEvent->pos());
  DescriptivePhrasePtr dp = this->getModel()->getItem(dropIdx);
  smtk::model::Group group;
  if (dp && (group = dp->relatedEntity().as<smtk::model::Group>()).isValid())
//  if(dp && dp->relatedEntity().isGroup() )
    {
    BitFlags ef = (dp->relatedEntity().entityFlags() & DIMENSION_2) ?
      CELL_2D : CELL_3D;
    foreach(QModelIndex sel, this->selectedIndexes())
      {
  //    DescriptivePhrasePtr childp = this->getModel()->getItem(sel);
  //    group.addEntity(childp->relatedEntity());
      this->recursiveSelect(qmodel->getItem(sel), selentityrefs, ef);
      }
//    EntityRefs entities;
//    EntityRef::EntityRefsFromUUIDs(entities, qmodel->manager(), ids);
    std::cout << selentityrefs.size() << " selentityrefs, " << selentityrefs.size() << " entities\n";

    group.addEntities(selentityrefs);
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
  smtk::model::EntityRefs selentityrefs;
  foreach(QModelIndex sel, this->selectedIndexes())
    {
    this->recursiveSelect(qmodel->getItem(sel), selentityrefs,
      CELL_ENTITY /*| SHELL_ENTITY  | GROUP_ENTITY | MODEL_ENTITY */);
    }

  emit this->entitiesSelected(selentityrefs);
}

//----------------------------------------------------------------------------
void qtModelView::recursiveSelect (smtk::model::DescriptivePhrasePtr dPhrase,
    smtk::model::EntityRefs& selentityrefs, BitFlags entityFlags)
{
  if(dPhrase &&
    (dPhrase->relatedEntity().entityFlags() & entityFlags)/* &&
    selentityrefs.find(dPhrase->relatedEntity()) == selentityrefs.end() */)
    {
    selentityrefs.insert(dPhrase->relatedEntity());
    }

  smtk::model::DescriptivePhrases sub = dPhrase->subphrases();
  for (smtk::model::DescriptivePhrases::iterator it = sub.begin();
    it != sub.end(); ++it)
    {
    this->recursiveSelect(*it, selentityrefs, entityFlags);
    }
}

//----------------------------------------------------------------------------
void qtModelView::selectEntities(const smtk::common::UUIDs& selEntities)
{
  smtk::model::QEntityItemModel* qmodel =
    dynamic_cast<smtk::model::QEntityItemModel*>(this->model());

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

void qtModelView::removeSelected()
{
  QEntityItemModel* qmodel = this->getModel();
  smtk::model::ManagerPtr pstore = qmodel->manager();
  Models models;
  smtk::model::EntityRef::EntityRefsFromUUIDs(
    models,
    pstore,
    pstore->entitiesMatchingFlags(smtk::model::MODEL_ENTITY));
  if(models.empty())
    {
    return;
    }

  // remove the groups later, in case some of the group's
  // children may also be selected.
  QList<smtk::model::Group> groups;
  foreach(QModelIndex sel, this->selectedIndexes())
    {
    DescriptivePhrasePtr dp = this->getModel()->getItem(sel);
    smtk::model::Group group;
    if(dp && (group = dp->relatedEntity().as<smtk::model::Group>()).isValid())
      {
      groups.append(group);
      }
    else
      {
      this->removeFromGroup(sel);
      }
    }

  foreach(smtk::model::Group group, groups)
    {
    models.begin()->removeGroup(group);
    pstore->erase(group.entity());
    }
}

void qtModelView::removeFromGroup(const QModelIndex& qidx)
{
  smtk::model::Group group;
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
  * Note that a group (EntityPhrase with a EntityRef whose isGroup() is true)
  * may contain an EntityListPhrase, each entry of which is in the group.
  * We must test for this 1 level of indirection as well as for direct
  * children.
  */
smtk::model::Group qtModelView::groupParentOfIndex(const QModelIndex& qidx)
{
  smtk::model::Group group;
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
        if (ephrase && (group = ephrase->relatedEntity().as<smtk::model::Group>()).isValid())
          return group; // direct child of a Group's summary phrase.
        EntityListPhrasePtr lphrase = smtk::dynamic_pointer_cast<EntityListPhrase>(phrase);
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

  QModelIndex dropIdx = this->indexAt(p);
  DescriptivePhrasePtr dp = this->getModel()->getItem(dropIdx);
  smtk::model::SessionRef brSession;

  if (dp && (brSession = dp->relatedEntity().as<smtk::model::SessionRef>()).isValid())
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
smtk::model::SessionRef qtModelView::getSessionRef(
  const QModelIndex &idx) const
{
  DescriptivePhrasePtr dp = this->getModel()->getItem(idx);
  smtk::model::SessionRef brSession;
  while (dp)
    {
    brSession = dp->relatedEntity().as<smtk::model::SessionRef>();
    if(brSession.isValid())
      break;
    dp = dp->parent();
    }
  return brSession;
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
  QDockWidget* opDock = this->operatorsDock(opName, session);
  if (!opDock)
    {
    std::cerr
      << "Could not create UI for operator: \"" << opName << "\" for session"
      << " \"" << session->name() << "\""
      << " (" << session->sessionId() << ")\n";
    return;
    }
  opDock->show();

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
QDockWidget* qtModelView::operatorsDock(
  const std::string& opName, smtk::model::SessionPtr session)
{
  QEntityItemModel* qmodel = this->getModel();
  smtk::model::ManagerPtr pstore = qmodel->manager();

  SessionRef bs(pstore, session->sessionId());

  if(this->m_OperatorsDock && this->m_OperatorsWidget)
    {
    this->m_OperatorsWidget->setCurrentOperation(opName, session);
    this->m_OperatorsDock->setWindowTitle(bs.flagSummary().c_str());
    return this->m_OperatorsDock;
    }

  qtModelOperationWidget* opWidget = new qtModelOperationWidget();
  if(!opWidget->setCurrentOperation(opName, session))
    {
    delete opWidget;
    return NULL;
    }
  QObject::connect(opWidget, SIGNAL(operationRequested(const smtk::model::OperatorPtr&)),
    this, SIGNAL(operationRequested(const smtk::model::OperatorPtr&)));
  QObject::connect(opWidget, SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)),
    this, SIGNAL(fileItemCreated(smtk::attribute::qtFileItem*)));

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
  dw->setWindowTitle(bs.flagSummary().c_str());
  dw->setObjectName("operatorsDockWidget");
  dw->setWidget(s);
  dw->setFloating(true);

  this->m_OperatorsWidget = opWidget;
  this->m_OperatorsDock = dw;
  return dw;
}

//----------------------------------------------------------------------------
OperatorPtr qtModelView::getSetPropertyOp(const QModelIndex& idx)
{
  smtk::model::SessionRef sref = this->getSessionRef(idx);
  if(!sref.isValid())
    {
    std::cerr
      << "Could not find session!\n";
    return OperatorPtr();
    }
  // create SetProperty op
  smtk::model::SessionPtr session = sref.session();
  return this->getSetPropertyOp(session);
}

//----------------------------------------------------------------------------
OperatorPtr qtModelView::getSetPropertyOp(smtk::model::SessionPtr session)
{
  OperatorPtr brOp = session->op("set property");
  if (!brOp)
    {
    std::cerr
      << "Could not create operator: \"" << "set property" << "\" for session"
      << " \"" << session->name() << "\""
      << " (" << session->sessionId() << ")\n";
    return OperatorPtr();
    }

  smtk::attribute::AttributePtr attrib = brOp->specification();
  if(!attrib->isValid())
    {
    std::cerr
      << "Invalid spec for the op: " << brOp->name() << "\n";
    return OperatorPtr();
    }

  attrib->system()->setRefModelManager(session->manager());

  return brOp;
}

//----------------------------------------------------------------------------
void qtModelView::toggleEntityVisibility( const QModelIndex& idx)
{
  OperatorPtr brOp = this->getSetPropertyOp(idx);
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
  OperatorPtr brOp = this->getSetPropertyOp(idx);
  if(!brOp || !brOp->specification()->isValid())
    return;
  smtk::model::EntityRefs selentityrefs;
  this->recursiveSelect(this->getModel()->getItem(idx), selentityrefs,
    CELL_ENTITY | SHELL_ENTITY  | GROUP_ENTITY |
    MODEL_ENTITY | INSTANCE_ENTITY | SESSION);
  DescriptivePhrasePtr dp = this->getModel()->getItem(idx);
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
    OperatorPtr brOp = this->getSetPropertyOp(session);
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
    OperatorPtr brOp = this->getSetPropertyOp(session);
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
