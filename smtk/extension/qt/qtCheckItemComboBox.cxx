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
