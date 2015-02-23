//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtMeshEntityItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/MeshEntityItem.h"
#include "smtk/attribute/MeshEntityItemDefinition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/common/UUID.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtCheckItemComboBox.h"

#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QVariant>
#include <QSizePolicy>
#include <QPointer>
#include <QTextEdit>
#include <QComboBox>
#include <QToolButton>
#include <QStandardItemModel>
#include <QAbstractItemView>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtMeshEntityItemInternals
{
public:

  QPointer<QGridLayout> EntryLayout;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;

//  QPointer<qtModelEntityItem> EntityAssociationItem;

  QPointer<QToolButton> GrowButton;
  QPointer<QToolButton> GrowPlusButton;
  QPointer<QToolButton> GrowMinusButton;
  QPointer<QToolButton> CancelButton;

};

//----------------------------------------------------------------------------
qtMeshEntityItem::qtMeshEntityItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p, qtBaseView* bview,
   Qt::Orientation enVectorItemOrient) : qtItem(dataObj, p, bview)
{
  this->Internals = new qtMeshEntityItemInternals;
  this->IsLeafItem = true;
  this->Internals->VectorItemOrient = enVectorItemOrient;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtMeshEntityItem::~qtMeshEntityItem()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtMeshEntityItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
}

//----------------------------------------------------------------------------
void qtMeshEntityItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if(!dataObj || !this->passAdvancedCheck() || (this->baseView() &&
    !this->baseView()->uiManager()->passItemCategoryCheck(
      dataObj->definition())))
    {
    return;
    }

  this->updateItemData();
}

//----------------------------------------------------------------------------
void qtMeshEntityItem::updateItemData()
{
  this->updateUI();
  this->qtItem::updateItemData();
}

QToolButton* internal_createToolButton(
  const QString& strIconName, const QString& strToolTip, QWidget* pw)
{
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QToolButton* retButton = new QToolButton(pw);
  retButton->setToolTip(strToolTip);
  retButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  retButton->setFixedSize(QSize(16, 16));
  retButton->setIcon(QIcon(strIconName));
  retButton->setSizePolicy(sizeFixedPolicy);
  return retButton;
}

//----------------------------------------------------------------------------
void qtMeshEntityItem::addMeshOpButtons()
{
/*
  this->Internals->EntityAssociationItem =
    qtAttribute::createModelEntityItem(
        item, this->Widget, this->baseView(),
        this->Internals->VectorItemOrient);
  this->Internals->EntityAssociationItem->setLabelVisible(false);
*/

  QBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->setMargin(0);
  buttonLayout->setSpacing(3);

  // grow button
  this->Internals->GrowButton = internal_createToolButton(
    ":/icons/attribute/growcell32.png",
    "Grow Selection on Associated Entities", this->Widget);
  connect(this->Internals->GrowButton, SIGNAL(clicked()),
    this, SLOT(onRequestValuesUpdate()));

  // grow plus button
  this->Internals->GrowPlusButton = internal_createToolButton(
    ":/icons/attribute/growplus32.png",
    "Grow and Append Selection on Selected Entities", this->Widget);
  connect(this->Internals->GrowPlusButton, SIGNAL(clicked()),
    this, SLOT(onRequestValuesUpdate()));

  // grow minus button
  this->Internals->GrowMinusButton = internal_createToolButton(
    ":/icons/attribute/growminus32.png",
    "Grow and Remove Selection on Selected Entities", this->Widget);
  connect(this->Internals->GrowMinusButton, SIGNAL(clicked()),
    this, SLOT(onRequestValuesUpdate()));

  // grow minus button
  this->Internals->CancelButton = internal_createToolButton(
    ":/icons/attribute/cancel32.png",
    "Candel Grow Selection Mode", this->Widget);
  connect(this->Internals->CancelButton, SIGNAL(clicked()),
    this, SLOT(onRequestValuesUpdate()));

  buttonLayout->addWidget(this->Internals->GrowButton);
  buttonLayout->addWidget(this->Internals->GrowPlusButton);
  buttonLayout->addWidget(this->Internals->GrowMinusButton);
  buttonLayout->addWidget(this->Internals->CancelButton);

  this->Internals->EntryLayout->addLayout(buttonLayout, 0, 1);
}

//----------------------------------------------------------------------------
void qtMeshEntityItem::updateUI()
{
  //smtk::attribute::ItemPtr dataObj = this->getObject();
  smtk::attribute::MeshEntityItemPtr dataObj =
    dynamic_pointer_cast<MeshEntityItem>(this->getObject());
  if(!dataObj || !this->passAdvancedCheck() || (this->baseView() &&
    !this->baseView()->uiManager()->passItemCategoryCheck(
      dataObj->definition())))
    {
    return;
    }

  this->Widget = new QFrame(this->parentWidget());
  this->Internals->EntryLayout = new QGridLayout(this->Widget);
  this->Internals->EntryLayout->setMargin(0);
  this->Internals->EntryLayout->setSpacing(0);
  this->Internals->EntryLayout->setAlignment( Qt::AlignLeft | Qt::AlignTop );

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if(dataObj->isOptional())
    {
    QCheckBox* optionalCheck = new QCheckBox(this->parentWidget());
    optionalCheck->setChecked(dataObj->isEnabled());
    optionalCheck->setText(" ");
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = optionalCheck->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)),
      this, SLOT(setOutputOptional(int)));
    labelLayout->addWidget(optionalCheck);
    }
  smtk::attribute::MeshEntityItemPtr item =
    dynamic_pointer_cast<MeshEntityItem>(dataObj);
  const MeshEntityItemDefinition *itemDef =
    dynamic_cast<const MeshEntityItemDefinition*>(dataObj->definition().get());

  QString labelText;
  if(!item->label().empty())
    {
    labelText = item->label().c_str();
    }
  else
    {
    labelText = item->name().c_str();
    }
  QLabel* label = new QLabel(labelText, this->Widget);
  label->setSizePolicy(sizeFixedPolicy);
  if(this->baseView())
    {
    label->setFixedWidth(this->baseView()->fixedLabelWidth() - padding);
    }
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

//  qtOverlayFilter *filter = new qtOverlayFilter(this);
//  label->installEventFilter(filter);

  // add in BriefDescription as tooltip if available
  const std::string strBriefDescription = itemDef->briefDescription();
  if(!strBriefDescription.empty())
    {
    label->setToolTip(strBriefDescription.c_str());
    }

  if(itemDef->advanceLevel() && this->baseView())
    {
    label->setFont(this->baseView()->uiManager()->advancedFont());
    }
  labelLayout->addWidget(label);
  this->Internals->theLabel = label;

  this->addMeshOpButtons();

  this->Internals->EntryLayout->addLayout(labelLayout, 0, 0);
//  layout->addWidget(this->Internals->EntryFrame, 0, 1);
  if(this->parentWidget() && this->parentWidget()->layout())
    {
    this->parentWidget()->layout()->addWidget(this->Widget);
    }
  if(dataObj->isOptional())
    {
    this->setOutputOptional(dataObj->isEnabled() ? 1 : 0);
    }
}

//----------------------------------------------------------------------------
void qtMeshEntityItem::setOutputOptional(int state)
{
  smtk::attribute::MeshEntityItemPtr item =
    dynamic_pointer_cast<MeshEntityItem>(this->getObject());
  if(!item)
    {
    return;
    }
  bool enable = state ? true : false;
//  this->Internals->EntryFrame->setEnabled(enable);
  if(enable != this->getObject()->isEnabled())
    {
    this->getObject()->setIsEnabled(enable);
    if(this->baseView())
      this->baseView()->valueChanged(this->getObject());
    }
}

//----------------------------------------------------------------------------
void qtMeshEntityItem::updateValues(const std::set<int> vals,
        MeshListUpdateType opType)
{
  smtk::attribute::MeshEntityItemPtr meshEntityItem =
    dynamic_pointer_cast<MeshEntityItem>(this->getObject());
  if(!meshEntityItem)
    {
    return;
    }
  switch(opType)
  {
    case RESET:
      meshEntityItem->setValues(vals);
      break;
    case APPEND:
      meshEntityItem->insertValues(vals);
      break;
    case SUBTRACT:
      meshEntityItem->removeValues(vals);
      break;
    case CANCEL:
      meshEntityItem->reset();
      break;
    default:
      std::cerr << "ERROR: Unrecognized MeshListUpdateType: "
                << opType << std::endl;
      break;
  }
}

//----------------------------------------------------------------------------
void qtMeshEntityItem::onRequestValuesUpdate()
{
  QToolButton* const cButton = qobject_cast<QToolButton*>(
    QObject::sender());
  if(!cButton)
    {
    return;
    }
  MeshListUpdateType upType;
  if(cButton == this->Internals->GrowButton)
    upType = RESET;
  else if(cButton == this->Internals->GrowPlusButton)
    upType = APPEND;
  else if(cButton == this->Internals->GrowMinusButton)
    upType = SUBTRACT;
  else if(cButton == this->Internals->CancelButton)
    upType = CANCEL;
  else
    {
    std::cerr << "ERROR: Unrecognized button click "
              << cButton->objectName().toStdString() << std::endl;
    return;
    }

  smtk::attribute::MeshEntityItemPtr meshEntityItem =
    dynamic_pointer_cast<MeshEntityItem>(this->getObject());
  if(!meshEntityItem)
    {
    return;
    }
  const MeshEntityItemDefinition *itemDef =
    dynamic_cast<const MeshEntityItemDefinition*>(meshEntityItem->definition().get());

  smtk::attribute::AttributePtr att = meshEntityItem->attribute();
  smtk::attribute::ModelEntityItem::Ptr modelEntities =
    att->findModelEntity(itemDef->refModelEntityName());
  if(modelEntities)
    emit this->requestValuesUpdate(modelEntities, upType);
}
