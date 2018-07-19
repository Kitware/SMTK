//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtVoidItem.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QCheckBox>
#include <QPointer>
#include <QSizePolicy>
#include <QVBoxLayout>

#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/VoidItemDefinition.h"

using namespace smtk::extension;

class qtVoidItemInternals
{
public:
  QPointer<QCheckBox> optionalCheck;
};

qtItem* qtVoidItem::createItemWidget(const AttributeItemInfo& info)
{
  return new qtVoidItem(info);
}

qtVoidItem::qtVoidItem(const AttributeItemInfo& info)
  : qtItem(info)
{
  this->Internals = new qtVoidItemInternals;
  m_isLeafItem = true;
  this->createWidget();
}

qtVoidItem::~qtVoidItem()
{
  delete this->Internals;
}

void qtVoidItem::setLabelVisible(bool visible)
{
  smtk::attribute::ItemPtr dataObj = m_itemInfo.item();
  if (!dataObj)
  {
    return;
  }

  QString txtLabel = dataObj->label().empty() ? dataObj->name().c_str() : dataObj->label().c_str();

  this->Internals->optionalCheck->setText(visible ? txtLabel : "");
}

void qtVoidItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = m_itemInfo.item();
  if (!dataObj || !this->passAdvancedCheck() ||
    (m_itemInfo.uiManager() &&
      !m_itemInfo.uiManager()->passItemCategoryCheck(dataObj->definition())))
  {
    return;
  }

  this->clearChildItems();
  m_widget = new QFrame(this->parentWidget());
  new QVBoxLayout(m_widget);
  m_widget->layout()->setMargin(0);
  m_widget->layout()->setSpacing(0);

  QCheckBox* optionalCheck = new QCheckBox(m_widget);
  optionalCheck->setChecked(dataObj->definition()->isEnabledByDefault());
  QSizePolicy sizeFixedPolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  optionalCheck->setSizePolicy(sizeFixedPolicy);
  QString txtLabel = dataObj->label().empty() ? dataObj->name().c_str() : dataObj->label().c_str();

  if (dataObj->advanceLevel() > 0)
  {
    optionalCheck->setFont(m_itemInfo.uiManager()->advancedFont());
  }
  if (dataObj->definition()->briefDescription().length())
  {
    optionalCheck->setToolTip(dataObj->definition()->briefDescription().c_str());
  }

  optionalCheck->setText(txtLabel);
  QObject::connect(optionalCheck, SIGNAL(stateChanged(int)), this, SLOT(setOutputOptional(int)));
  this->Internals->optionalCheck = optionalCheck;
  m_widget->layout()->addWidget(this->Internals->optionalCheck);
  this->updateItemData();
}

void qtVoidItem::updateItemData()
{
  smtk::attribute::ItemPtr dataObj = m_itemInfo.item();
  if (!dataObj || !this->Internals->optionalCheck)
  {
    return;
  }
  this->Internals->optionalCheck->setChecked(dataObj->isEnabled());
  this->qtItem::updateItemData();
}

void qtVoidItem::setOutputOptional(int state)
{
  bool enable = state ? true : false;
  auto item = m_itemInfo.item();
  if (enable != item->isEnabled())
  {
    item->setIsEnabled(enable);
    m_itemInfo.baseView()->valueChanged(item);
    emit this->modified();
  }
}
