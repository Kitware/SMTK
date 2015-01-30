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

#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/model/Cursor.h"
#include "smtk/model/Manager.h"

#include <QAbstractItemView>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <QStandardItem>

using namespace smtk::attribute;

qtCheckableComboItemDelegate::qtCheckableComboItemDelegate(QWidget* owner) :
  QStyledItemDelegate(owner)
{
}

void qtCheckableComboItemDelegate::paint(QPainter * painter_, const QStyleOptionViewItem & option_, const QModelIndex & index_) const
{
    QStyleOptionViewItem & refToNonConstOption = const_cast<QStyleOptionViewItem &>(option_);
    refToNonConstOption.showDecorationSelected = false;
    //refToNonConstOption.state &= ~QStyle::State_HasFocus & ~QStyle::State_MouseOver;

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
  if(itemModel)
    {
    for(int row=1; row<this->count(); row++)
      {
      if(itemModel->item(row)->checkState() == Qt::Checked)
        {
        numSel++;
        }
      }
    }
  QString displayText = QString::number(numSel) + " " + m_displayTextExt;
  this->m_displayItem->setText(displayText);
  this->view()->model()->setData(this->view()->model()->index(0,0),
    displayText, Qt::DisplayRole);
  this->view()->update();
}

void qtCheckItemComboBox::hidePopup()
{
  this->QComboBox::hidePopup();
  this->setCurrentIndex(0);
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
  int row=1;
  smtk::model::Cursors modelEnts = modelManager->entitiesMatchingFlagsAs<smtk::model::Cursors>(
    itemDef->membershipMask(), false);
  for(smtk::model::Cursors::iterator it = modelEnts.begin(); it != modelEnts.end(); ++it)
    {
    if((*it).isUseEntity())
      continue;
    QStandardItem* item = new QStandardItem;
    std::string entName = (*it).name();
    item->setText(entName.c_str());
    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    //item->setData(this->Internals->AttSelections[keyName], Qt::CheckStateRole);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    item->setCheckable(true);
    item->setCheckState(ModelEntityItem->has(*it) ? Qt::Checked : Qt::Unchecked);
    
    item->setData((*it).entity().toString().c_str(), Qt::UserRole);
    itemModel->insertRow(row, item);
    }
  itemModel->sort(0);

  connect(this->model(),
    SIGNAL(dataChanged ( const QModelIndex&, const QModelIndex&)),
    this, SLOT(itemCheckChanged(const QModelIndex&, const QModelIndex&)));

  //connect(this->Internals->checkableAttComboModel, SIGNAL(itemChanged ( QStandardItem*)),
  //  this, SLOT(attributeFilterChanged(QStandardItem*)));
  this->blockSignals(false);
  this->updateText();
  this->hidePopup();
}

//-----------------------------------------------------------------------------
void qtModelEntityItemCombo::showPopup()
{
  this->init();
  this->qtCheckItemComboBox::showPopup();
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
    smtk::model::Cursor selcursor(
      ModelEntityItem->attribute()->system()->refModelManager(), entid.toStdString());
    if(item->checkState() == Qt::Checked)
      {
      bool success = false;
      // find an un-set index, and set the value
      for(std::size_t idx=0;
        idx < ModelEntityItem->numberOfValues(); ++idx)
        {
        if(!ModelEntityItem->isSet(idx))
          {
          success = ModelEntityItem->setValue(idx, selcursor);
          break;
          }
        }

      if(!success)
        {
        success = ModelEntityItem->appendValue(selcursor);
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
      std::ptrdiff_t idx = ModelEntityItem->find(selcursor);
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
