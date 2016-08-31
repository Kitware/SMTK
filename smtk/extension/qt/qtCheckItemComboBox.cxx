//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtCheckItemComboBox.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshItemDefinition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include <QAbstractItemView>
#include <QStandardItemModel>
#include <QStyleOptionButton>
#include <QStyleOptionViewItem>
#include <QStandardItem>
#include <QMouseEvent>

using namespace smtk::attribute;
using namespace smtk::extension;

qtCheckableComboItemDelegate::qtCheckableComboItemDelegate(QWidget* owner) :
  QStyledItemDelegate(owner)
{
}

void qtCheckableComboItemDelegate::paint(QPainter * painter_,
					 const QStyleOptionViewItem & option_,
					 const QModelIndex & index_) const
{
    QStyleOptionViewItem & refToNonConstOption = const_cast<QStyleOptionViewItem &>(option_);
    refToNonConstOption.showDecorationSelected = false;
//    refToNonConstOption.state &= QStyle::SH_ItemView_MovementWithoutUpdatingSelection;

    QStyledItemDelegate::paint(painter_, refToNonConstOption, index_);
}

qtCheckItemComboBox::qtCheckItemComboBox(QWidget* pw, const QString& displayExt) :
  QComboBox(pw), m_displayItem(NULL), m_displayTextExt(displayExt)
{
  this->setStyleSheet("combobox-popup: 0;");
  this->setMaxVisibleItems(10);
}

void qtCheckItemComboBox::init()
{
  this->m_displayItem = new QStandardItem;
  this->m_displayItem->setFlags(Qt::ItemIsEnabled);
  this->m_displayItem->setText("0 " + m_displayTextExt);
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel*>(this->model());
  if(itemModel)
    {
    itemModel->insertRow(0, this->m_displayItem);
    }
}

void qtCheckItemComboBox::updateText()
{
  int numSel = 0;
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel*>(this->model());
  QStandardItem* lastSelItem = NULL;
  if(itemModel)
    {
    for(int row=1; row<this->count(); row++)
      {
      if(itemModel->item(row)->checkState() == Qt::Checked)
        {
        lastSelItem = itemModel->item(row);
        numSel++;
        }
      }
    }
  QString displayText = (numSel == 1 && lastSelItem) ?
    lastSelItem->text() : QString::number(numSel) + " " + m_displayTextExt;
  this->m_displayItem->setText(displayText);
  this->view()->model()->setData(this->view()->model()->index(0,0),
    displayText, Qt::DisplayRole);
  this->view()->update();
}

void qtCheckItemComboBox::hidePopup()
{
  this->view()->clearSelection();
  this->QComboBox::hidePopup();
  this->setCurrentIndex(0);
}

//-----------------------------------------------------------------------------
void qtCheckItemComboBox::showPopup()
{
  this->view()->updateGeometry();
  this->QComboBox::showPopup();
}

//-----------------------------------------------------------------------------
qtModelEntityItemCombo::qtModelEntityItemCombo(
  smtk::attribute::ItemPtr entitem, QWidget * inParent, const QString& displayExt)
: qtCheckItemComboBox(inParent, displayExt), m_ModelEntityItem(entitem)
{
  this->setMinimumWidth(80);
}

//----------------------------------------------------------------------------
void qtModelEntityItemCombo::init()
{
  this->blockSignals(true);
  this->clear();
  this->qtCheckItemComboBox::init();
  this->model()->disconnect();

  if(!this->m_ModelEntityItem.lock())
    {
    this->blockSignals(false);
    return;
    }

  ModelEntityItemPtr ModelEntityItem =
    smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItem>(
    this->m_ModelEntityItem.lock());
  const ModelEntityItemDefinition *itemDef =
    static_cast<const ModelEntityItemDefinition *>(ModelEntityItem->definition().get());
  System *attSystem = ModelEntityItem->attribute()->system();
  smtk::model::ManagerPtr modelManager = attSystem->refModelManager();

  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel*>(this->model());
  // need to update the list, since it may be changed
  // We create a temporary group and use Group::meetsMembershipConstraints()
  // to test whether the mask allows association.
  smtk::model::Manager::Ptr tmpMgr = smtk::model::Manager::create();

  bool onlyGroups = (itemDef->membershipMask() & smtk::model::ENTITY_MASK)
                    == smtk::model::GROUP_ENTITY;
  smtk::model::Group tmpGrp = tmpMgr->addGroup();
  tmpGrp.setMembershipMask(itemDef->membershipMask());

  int row=1;
  if (modelManager)
    {
    for (smtk::model::UUIDWithEntity it = modelManager->topology().begin();
      it != modelManager->topology().end(); ++it)
      {

      smtk::model::EntityRef entref(modelManager, it->first);
      if (entref.isValid() && !entref.isUseEntity() &&
        // if the mask is only groups, get all groups from manager
        ((onlyGroups && entref.isGroup()) ||
         // else, check the membership constraints
         (!onlyGroups && tmpGrp.meetsMembershipConstraints(entref))))
        {
        QStandardItem* item = new QStandardItem;
        std::string entName = entref.name();
        item->setText(entName.c_str());
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        //item->setData(this->Internals->AttSelections[keyName], Qt::CheckStateRole);
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setCheckable(true);
        item->setCheckState(ModelEntityItem->has(entref) ? Qt::Checked : Qt::Unchecked);

        item->setData(entref.entity().toString().c_str(), Qt::UserRole);
        itemModel->insertRow(row, item);
        }
      }
    itemModel->sort(0);
    }

  connect(this->model(),
    SIGNAL(dataChanged ( const QModelIndex&, const QModelIndex&)),
    this, SLOT(itemCheckChanged(const QModelIndex&, const QModelIndex&)));

  //connect(this->Internals->checkableAttComboModel, SIGNAL(itemChanged ( QStandardItem*)),
  //  this, SLOT(attributeFilterChanged(QStandardItem*)));
  this->blockSignals(false);
  this->view()->viewport()->installEventFilter(this);
  // this->view()->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->updateText();
  this->hidePopup();
}

//-----------------------------------------------------------------------------
void qtModelEntityItemCombo::showPopup()
{
  this->init();
  this->qtCheckItemComboBox::showPopup();
}

//-----------------------------------------------------------------------------
bool qtModelEntityItemCombo::eventFilter(QObject* editor, QEvent* evt)
{
  if(evt->type()==QEvent::MouseButtonRelease)
    {
    int index = view()->currentIndex().row();
/*
    // with the help of styles, check if checkbox rect contains 'pos'
    QMouseEvent* e = dynamic_cast<QMouseEvent*>(evt);
    QStyleOptionButton opt;
    opt.rect = view()->visualRect(view()->currentIndex());
    QRect r = style()->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt);
    if(r.contains(e->pos()))
      {
*/
      if (itemData(index, Qt::CheckStateRole) == Qt::Checked)
        setItemData(index, Qt::Unchecked, Qt::CheckStateRole);
      else
        setItemData(index, Qt::Checked, Qt::CheckStateRole);
//      }
    return true;
    }

  return QObject::eventFilter(editor, evt);
}

//----------------------------------------------------------------------------
void qtModelEntityItemCombo::itemCheckChanged(
  const QModelIndex& topLeft, const QModelIndex& )
{
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel*>(this->model());
  QStandardItem* item = itemModel->item(topLeft.row());
  if(!item)
    {
    return;
    }
  ModelEntityItemPtr ModelEntityItem =
    smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItem>(
    this->m_ModelEntityItem.lock());
  const ModelEntityItemDefinition *itemDef =
    static_cast<const ModelEntityItemDefinition *>(ModelEntityItem->definition().get());
  QString entid = item->data(Qt::UserRole).toString();
  if(!entid.isEmpty())
    {
    smtk::model::EntityRef selentityref(
      ModelEntityItem->attribute()->system()->refModelManager(), entid.toStdString());
    if(item->checkState() == Qt::Checked)
      {
      bool success = false;
      // find an un-set index, and set the value
      for(std::size_t idx=0;
        idx < ModelEntityItem->numberOfValues(); ++idx)
        {
        if(!ModelEntityItem->value(idx).isValid())
          {
          success = ModelEntityItem->setValue(idx, selentityref);
          break;
          }
        }

      if(!success)
        {
        success = ModelEntityItem->appendValue(selentityref);
        if(!success)
          {
          this->blockSignals(true);
          item->setCheckState(Qt::Unchecked);
          this->blockSignals(false);
          }
        }
      }
    else
      {
      std::ptrdiff_t idx = ModelEntityItem->find(selentityref);
      if(idx >=0)
        {
        if(itemDef->isExtensible())
          ModelEntityItem->removeValue(idx);
        else
          ModelEntityItem->unset(idx);
        }
      }
    this->updateText();
    }
}

//-----------------------------------------------------------------------------
qtMeshItemCombo::qtMeshItemCombo(
  smtk::attribute::ItemPtr entitem, QWidget * inParent, const QString& displayExt)
: qtCheckItemComboBox(inParent, displayExt), m_MeshItem(entitem)
{
  this->setMinimumWidth(80);
}

//----------------------------------------------------------------------------
void qtMeshItemCombo::init()
{
  this->blockSignals(true);
  this->clear();
  this->qtCheckItemComboBox::init();
  this->model()->disconnect();

  if(!this->m_MeshItem.lock())
    {
    this->blockSignals(false);
    return;
    }

  MeshItemPtr meshItem =
    smtk::dynamic_pointer_cast<smtk::attribute::MeshItem>(
    this->m_MeshItem.lock());
  System *attSystem = meshItem->attribute()->system();
  smtk::model::ManagerPtr modelManager = attSystem->refModelManager();

  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel*>(this->model());

  if (modelManager)
    {
    // find out all assoicated collections, or use all collections if none associated
    smtk::common::UUIDs collectionIds;
    smtk::model::EntityRefs associatedEnts =
      meshItem->attribute()->associatedModelEntities<smtk::model::EntityRefs>();
    smtk::model::EntityRefs::const_iterator it;
    for(it = associatedEnts.begin(); it != associatedEnts.end(); ++it)
      {
      smtk::common::UUIDs uuids = modelManager->meshes()->associatedCollectionIds(*it);
      collectionIds.insert(uuids.begin(), uuids.end());
      }
    std::vector<smtk::mesh::CollectionPtr> collections;
    smtk::common::UUIDs::const_iterator uit;
    for(uit = collectionIds.begin(); uit != collectionIds.end(); ++uit)
      {
      collections.push_back(modelManager->meshes()->collection(*uit));
      }
    // if no entities assoicated, use all collections with associations
    if(associatedEnts.size() == 0)
      {
      collections = modelManager->meshes()->collectionsWithAssociations();
      }

    smtk::mesh::MeshSets availableMeshes;
    int row = 1;
    for (std::vector<smtk::mesh::CollectionPtr>::const_iterator cit =
        collections.begin(); cit != collections.end(); ++cit)
      {
      if ((*cit)->isValid())
        {
        QStandardItem* item = new QStandardItem;
        std::string meshName = (*cit)->name();
        item->setText(meshName.c_str());
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        //item->setData(this->Internals->AttSelections[keyName], Qt::CheckStateRole);
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
        item->setCheckable(true);
        smtk::mesh::MeshSet amesh = (*cit)->meshes();
        availableMeshes.insert(amesh);
        item->setCheckState( meshItem->hasValue(amesh) ? Qt::Checked : Qt::Unchecked);

        item->setData((*cit)->entity().toString().c_str(), Qt::UserRole);
        itemModel->insertRow(row, item);
        }
      }
    itemModel->sort(0);
    // for any assigned value in the MeshItem, it has to be in the availableMeshes list
    for(std::size_t i=0; i<meshItem->numberOfValues(); ++i)
      {
      if(meshItem->isSet(i))
        {
        smtk::mesh::MeshSet amesh = meshItem->value(i);
        if(availableMeshes.find(amesh) == availableMeshes.end())
          {
          meshItem->unset(i);
          }
        }
      }

    }

  connect(this->model(),
    SIGNAL(dataChanged ( const QModelIndex&, const QModelIndex&)),
    this, SLOT(itemCheckChanged(const QModelIndex&, const QModelIndex&)));

  //connect(this->Internals->checkableAttComboModel, SIGNAL(itemChanged ( QStandardItem*)),
  //  this, SLOT(attributeFilterChanged(QStandardItem*)));
  this->blockSignals(false);
  this->view()->viewport()->installEventFilter(this);
  // this->view()->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->updateText();
  this->hidePopup();
}

//-----------------------------------------------------------------------------
void qtMeshItemCombo::showPopup()
{
  this->init();
  this->qtCheckItemComboBox::showPopup();
}

//-----------------------------------------------------------------------------
bool qtMeshItemCombo::eventFilter(QObject* editor, QEvent* evt)
{
  if(evt->type()==QEvent::MouseButtonRelease)
    {
    int index = view()->currentIndex().row();
/*
    // with the help of styles, check if checkbox rect contains 'pos'
    QMouseEvent* e = dynamic_cast<QMouseEvent*>(evt);
    QStyleOptionButton opt;
    opt.rect = view()->visualRect(view()->currentIndex());
    QRect r = style()->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt);
    if(r.contains(e->pos()))
      {
*/
      if (itemData(index, Qt::CheckStateRole) == Qt::Checked)
        setItemData(index, Qt::Unchecked, Qt::CheckStateRole);
      else
        setItemData(index, Qt::Checked, Qt::CheckStateRole);
//      }
    return true;
    }

  return QObject::eventFilter(editor, evt);
}

//----------------------------------------------------------------------------
void qtMeshItemCombo::itemCheckChanged(
  const QModelIndex& topLeft, const QModelIndex& )
{
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel*>(this->model());
  QStandardItem* item = itemModel->item(topLeft.row());
  if(!item)
    {
    return;
    }
  QString strcollectionid = item->data(Qt::UserRole).toString();
  if(strcollectionid.isEmpty())
    {
    return;
    }
  MeshItemPtr meshItem =
    smtk::dynamic_pointer_cast<smtk::attribute::MeshItem>(
    this->m_MeshItem.lock());
  const MeshItemDefinition *itemDef =
    static_cast<const MeshItemDefinition *>(meshItem->definition().get());
  smtk::common::UUID collectionid(strcollectionid.toStdString());
  smtk::mesh::CollectionPtr selcollection = meshItem->attribute()->
    system()->refModelManager()->meshes()->collection(collectionid);
  if(selcollection && selcollection->isValid())
    {
    smtk::mesh::MeshSet allmeshes = selcollection->meshes();
    if(item->checkState() == Qt::Checked)
      {
      bool success = false;
      // find an un-set index, and set the value
      for(std::size_t idx=0;
        idx < meshItem->numberOfValues(); ++idx)
        {
        if(meshItem->value(idx).is_empty())
          {
          success = meshItem->setValue(idx, allmeshes);
          break;
          }
        }

      if(!success)
        {
        success = meshItem->appendValue(allmeshes);
        if(!success)
          {
          this->blockSignals(true);
          item->setCheckState(Qt::Unchecked);
          this->blockSignals(false);
          }
        }
      }
    else
      {
      std::ptrdiff_t idx = meshItem->find(allmeshes);
      if(idx >=0)
        {
        if(itemDef->isExtensible())
          meshItem->removeValue(idx);
        else
          meshItem->unset(idx);
        }
      }
    this->updateText();
    }
}
