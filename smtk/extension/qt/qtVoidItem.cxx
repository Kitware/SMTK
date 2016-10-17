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
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtBaseView.h"

#include <QCheckBox>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QPointer>

#include "smtk/attribute/VoidItem.h"
#include "smtk/attribute/VoidItemDefinition.h"

using namespace smtk::extension;

//----------------------------------------------------------------------------
class qtVoidItemInternals
{
public:
  QPointer<QCheckBox> optionalCheck;
};

//----------------------------------------------------------------------------
qtVoidItem::qtVoidItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p, qtBaseView* bview) :
   qtItem(dataObj, p, bview)
{
  this->Internals = new qtVoidItemInternals;
  this->IsLeafItem = true;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtVoidItem::~qtVoidItem()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtVoidItem::setLabelVisible(bool visible)
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if(!dataObj)
    {
    return;
    }

  QString txtLabel = dataObj->label().empty() ?
     dataObj->name().c_str() : dataObj->label().c_str();

  this->Internals->optionalCheck->setText(visible ? txtLabel : "");
}

//----------------------------------------------------------------------------
void qtVoidItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if(!dataObj || !this->passAdvancedCheck() || (this->baseView() &&
    !this->baseView()->uiManager()->passItemCategoryCheck(
      dataObj->definition())))
    {
    return;
    }

  this->clearChildItems();
  this->Widget = new QFrame(this->parentWidget());
  new QVBoxLayout(this->Widget);
  this->Widget->layout()->setMargin(0);
  this->Widget->layout()->setSpacing(0);

  QCheckBox* optionalCheck = new QCheckBox(this->Widget);
  optionalCheck->setChecked(dataObj->definition()->isEnabledByDefault());
  QSizePolicy sizeFixedPolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  optionalCheck->setSizePolicy(sizeFixedPolicy);
  QString txtLabel = dataObj->label().empty() ?
     dataObj->name().c_str() : dataObj->label().c_str();

  if(dataObj->advanceLevel() >0)
    {
    optionalCheck->setFont(this->baseView()->uiManager()->advancedFont());
    }
  if(dataObj->definition()->briefDescription().length())
    {
    optionalCheck->setToolTip(dataObj->definition()->briefDescription().c_str());
    }

  optionalCheck->setText(txtLabel);
  QObject::connect(optionalCheck, SIGNAL(stateChanged(int)),
    this, SLOT(setOutputOptional(int)));
  this->Internals->optionalCheck = optionalCheck;
  this->Widget->layout()->addWidget(this->Internals->optionalCheck);
  this->updateItemData();
}

//----------------------------------------------------------------------------
void qtVoidItem::updateItemData()
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if(!dataObj || !this->Internals->optionalCheck)
    {
    return;
    }
  this->Internals->optionalCheck->setChecked(dataObj->isEnabled());
  this->qtItem::updateItemData();
}
//----------------------------------------------------------------------------
void qtVoidItem::setOutputOptional(int state)
{
  bool enable = state ? true : false;
  if(enable != this->getObject()->isEnabled())
    {
    this->getObject()->setIsEnabled(enable);
    this->baseView()->valueChanged(this->getObject());
    emit this->modified();
    }
}
