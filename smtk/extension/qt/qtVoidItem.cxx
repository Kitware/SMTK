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
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QCheckBox>
#include <QLabel>
#include <QPointer>
#include <QSizePolicy>
#include <QVBoxLayout>

#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/VoidItemDefinition.h"
#include "smtk/simulation/UserData.h"

using namespace smtk::extension;

class qtVoidItemInternals
{
public:
  QPointer<QCheckBox> optionalCheck;
};

qtItem* qtVoidItem::createItemWidget(const qtAttributeItemInfo& info)
{
  return new qtVoidItem(info);
}

qtVoidItem::qtVoidItem(const qtAttributeItemInfo& info)
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

  QString txtLabel = dataObj->label().c_str();

  this->Internals->optionalCheck->setText(visible ? txtLabel : "");
}

void qtVoidItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = m_itemInfo.item();
  auto* iview = m_itemInfo.baseView();
  if (iview && !iview->displayItem(dataObj))
  {
    return;
  }

  this->clearChildItems();
  m_widget = new QFrame(this->parentWidget());
  m_widget->setObjectName(dataObj->name().c_str());
  if (this->isReadOnly())
  {
    m_widget->setEnabled(false);
  }
  auto* vLayout = new QVBoxLayout(m_widget);
  vLayout->setObjectName("vLayout");
  vLayout->setMargin(0);
  vLayout->setSpacing(0);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QString txtLabel = dataObj->label().c_str();
  bool labelIsBlank = txtLabel.trimmed().isEmpty();
  // If the item is optional then use a check-box else use a label
  if (dataObj->isOptional())
  {
    QCheckBox* optionalCheck = new QCheckBox(m_widget);
    optionalCheck->setObjectName("optionalCheck");

    // Check for "no_focus" user data
    auto udata = dataObj->userData("smtk.extensions.void_item.no_focus");
    if (udata != nullptr)
    {
      optionalCheck->setFocusPolicy(Qt::NoFocus);
    }

    optionalCheck->setChecked(dataObj->definition()->isEnabledByDefault());
    optionalCheck->setSizePolicy(sizeFixedPolicy);

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
    if (labelIsBlank)
    {
      this->setLabelVisible(false);
    }

    m_widget->layout()->addWidget(this->Internals->optionalCheck);
    this->updateItemData();
  }
  else if (!labelIsBlank)
  {
    auto* label = new QLabel(m_widget);
    label->setObjectName("label");
    label->setSizePolicy(sizeFixedPolicy);
    if (dataObj->advanceLevel() > 0)
    {
      label->setFont(m_itemInfo.uiManager()->advancedFont());
    }
    if (dataObj->definition()->briefDescription().length())
    {
      label->setToolTip(dataObj->definition()->briefDescription().c_str());
    }

    label->setText(txtLabel);
    m_widget->layout()->addWidget(label);
  }
}

void qtVoidItem::updateItemData()
{
  smtk::attribute::ItemPtr dataObj = m_itemInfo.item();
  if (!dataObj || !this->Internals->optionalCheck)
  {
    return;
  }
  this->Internals->optionalCheck->setChecked(dataObj->localEnabledState());
  this->qtItem::updateItemData();
}

void qtVoidItem::setOutputOptional(int state)
{
  bool enable = state != 0;
  auto item = m_itemInfo.item();
  if (enable != item->localEnabledState())
  {
    item->setIsEnabled(enable);
    auto* iview = m_itemInfo.baseView();
    if (iview)
    {
      iview->valueChanged(item);
    }
    Q_EMIT this->modified();
  }
}
