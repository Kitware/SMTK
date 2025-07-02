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
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"

#include <QAbstractItemView>
#include <QMouseEvent>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyleOptionButton>
#include <QStyleOptionViewItem>

using namespace smtk::attribute;
using namespace smtk::extension;

qtCheckableComboItemDelegate::qtCheckableComboItemDelegate(QWidget* owner)
  : QStyledItemDelegate(owner)
{
}

void qtCheckableComboItemDelegate::paint(
  QPainter* painter_,
  const QStyleOptionViewItem& option_,
  const QModelIndex& index_) const
{
  QStyleOptionViewItem& refToNonConstOption = const_cast<QStyleOptionViewItem&>(option_);
  refToNonConstOption.showDecorationSelected = false;
  //    refToNonConstOption.state &= QStyle::SH_ItemView_MovementWithoutUpdatingSelection;

  QStyledItemDelegate::paint(painter_, refToNonConstOption, index_);
}

qtCheckItemComboBox::qtCheckItemComboBox(QWidget* pw, const QString& displayExt)
  : QComboBox(pw)
  , m_displayTextExt(displayExt)
{
  this->setStyleSheet("combobox-popup: 0;");
  this->setMaxVisibleItems(10);
}

void qtCheckItemComboBox::init()
{
  m_displayItem = new QStandardItem;
  m_displayItem->setFlags(Qt::ItemIsEnabled);
  m_displayItem->setText("0 " + m_displayTextExt);
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel*>(this->model());
  if (itemModel)
  {
    itemModel->insertRow(0, m_displayItem);
  }
}

void qtCheckItemComboBox::updateText()
{
  int numSel = 0;
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel*>(this->model());
  QStandardItem* lastSelItem = nullptr;
  if (itemModel)
  {
    for (int row = 1; row < this->count(); row++)
    {
      if (itemModel->item(row)->checkState() == Qt::Checked)
      {
        lastSelItem = itemModel->item(row);
        numSel++;
      }
    }
  }
  QString displayText = (numSel == 1 && lastSelItem)
    ? lastSelItem->text()
    : QString::number(numSel) + " " + m_displayTextExt;
  m_displayItem->setText(displayText);
  this->view()->model()->setData(this->view()->model()->index(0, 0), displayText, Qt::DisplayRole);

  // For the item list, all the painting operations take place in the viewport
  // (since it is a QAbstractItemView), so use its update() to reflect any changes.
  // http://doc.qt.io/qt-5/qabstractitemview.html#update
  this->view()->viewport()->update();

  // Additionally, update the baseclass to reflect the changes in the
  // actual combobox (selected item name and button).
  this->update();
}

void qtCheckItemComboBox::hidePopup()
{
  this->view()->clearSelection();
  this->QComboBox::hidePopup();
  this->setCurrentIndex(0);
}

void qtCheckItemComboBox::showPopup()
{
  this->view()->updateGeometry();
  this->QComboBox::showPopup();
}

bool qtCheckItemComboBox::eventFilter(QObject* editor, QEvent* evt)
{
  if (evt->type() == QEvent::MouseButtonRelease)
  {
    const int index = view()->currentIndex().row();
    auto* itemModel = qobject_cast<QStandardItemModel*>(this->model());
    QStandardItem* item = itemModel->item(index);
    if (item->isCheckable())
    {
      const auto newState =
        itemData(index, Qt::CheckStateRole) == Qt::Checked ? Qt::Unchecked : Qt::Checked;
      setItemData(index, newState, Qt::CheckStateRole);
    }

    return true;
  }

  return QObject::eventFilter(editor, evt);
}

void qtCheckItemComboBox::showEvent(QShowEvent* /*e*/)
{
  this->init();
}
