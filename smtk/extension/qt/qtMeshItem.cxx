//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtMeshItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/common/View.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtAttributeView.h"
#include "smtk/extension/qt/qtCheckItemComboBox.h"
#include "smtk/extension/qt/qtNewAttributeWidget.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QStandardItemModel>
#include <QVBoxLayout>

using namespace smtk::attribute;
using namespace smtk::extension;

class qtMeshItemInternals
{
public:
  QPointer<qtMeshItemCombo> MeshItemCombo;
  QPointer<QCheckBox> optionalCheck;
  QPointer<QGridLayout> EntryLayout;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;
};

qtMeshItem::qtMeshItem(smtk::attribute::ItemPtr dataObj, QWidget* p, qtBaseView* view,
  Qt::Orientation enVectorItemOrient)
  : qtItem(dataObj, p, view)
{
  this->Internals = new qtMeshItemInternals;
  this->IsLeafItem = true;
  this->Internals->VectorItemOrient = enVectorItemOrient;
  this->createWidget();
}

qtMeshItem::~qtMeshItem()
{
  delete this->Internals;
}

smtk::attribute::MeshItemPtr qtMeshItem::meshItem()
{
  return dynamic_pointer_cast<MeshItem>(this->getObject());
}

bool qtMeshItem::add(const smtk::mesh::MeshSet& val)
{
  if (this->meshItem()->appendValue(val))
  {
    emit this->modified();
    return true;
  }
  return false;
}

bool qtMeshItem::remove(const smtk::mesh::MeshSet& val)
{
  auto item = this->meshItem();
  auto idx = item->find(val);
  if (idx < 0)
  {
    return false;
  }

  if (item->isExtensible())
  {
    item->removeValue(idx);
  }
  else
  {
    item->unset(idx);
  }
  emit this->modified();
  return true;
}

void qtMeshItem::setLabelVisible(bool visible)
{
  if (this->Internals->theLabel)
  {
    this->Internals->theLabel->setVisible(visible);
    if (visible)
    {
      this->Widget->setMinimumWidth(250);
    }
    else
    {
      this->Widget->setMinimumWidth(150);
    }
  }
}

void qtMeshItem::createWidget()
{
  if (!this->getObject())
  {
    return;
  }

  this->updateItemData();
}

void qtMeshItem::updateItemData()
{
  smtk::attribute::MeshItemPtr item = dynamic_pointer_cast<MeshItem>(this->getObject());
  if (!item || !this->passAdvancedCheck() ||
    (this->baseView() && !this->baseView()->uiManager()->passItemCategoryCheck(item->definition())))
  {
    return;
  }

  this->Widget = new QFrame(this->parentWidget());
  this->Internals->EntryLayout = new QGridLayout(this->Widget);
  this->Internals->EntryLayout->setMargin(0);
  this->Internals->EntryLayout->setSpacing(0);
  this->Internals->EntryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if (item->isOptional())
  {
    QCheckBox* optionalCheck = new QCheckBox(this->parentWidget());
    optionalCheck->setChecked(item->isEnabled());
    optionalCheck->setText(" ");
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = optionalCheck->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)), this, SLOT(setOutputOptional(int)));
    labelLayout->addWidget(optionalCheck);
  }
  const MeshItemDefinition* itemDef =
    dynamic_cast<const MeshItemDefinition*>(item->definition().get());

  QString labelText;
  if (!item->label().empty())
  {
    labelText = item->label().c_str();
  }
  else
  {
    labelText = item->name().c_str();
  }
  QLabel* label = new QLabel(labelText, this->Widget);
  label->setSizePolicy(sizeFixedPolicy);
  if (this->baseView())
  {
    label->setFixedWidth(this->baseView()->fixedLabelWidth() - padding);
  }
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // add in BriefDescription as tooltip if available
  const std::string strBriefDescription = itemDef->briefDescription();
  if (!strBriefDescription.empty())
  {
    label->setToolTip(strBriefDescription.c_str());
  }

  if (itemDef->advanceLevel() && this->baseView())
  {
    label->setFont(this->baseView()->uiManager()->advancedFont());
  }
  labelLayout->addWidget(label);
  this->Internals->theLabel = label;

  this->loadAssociatedEntities();

  this->Internals->EntryLayout->addLayout(labelLayout, 0, 0);
  //  layout->addWidget(this->Internals->EntryFrame, 0, 1);
  if (this->parentWidget() && this->parentWidget()->layout())
  {
    this->parentWidget()->layout()->addWidget(this->Widget);
  }
  if (item->isOptional())
  {
    this->setOutputOptional(item->isEnabled() ? 1 : 0);
  }
  this->qtItem::updateItemData();
}

void qtMeshItem::setOutputOptional(int state)
{
  smtk::attribute::MeshItemPtr item = dynamic_pointer_cast<MeshItem>(this->getObject());
  if (!item)
  {
    return;
  }
  bool enable = state ? true : false;
  if (this->Internals->MeshItemCombo)
  {
    this->Internals->MeshItemCombo->setVisible(enable);
  }
  if (enable != this->getObject()->isEnabled())
  {
    this->getObject()->setIsEnabled(enable);
    emit this->modified();
    if (this->baseView())
    {
      this->baseView()->valueChanged(this->getObject());
    }
  }
}

void qtMeshItem::loadAssociatedEntities()
{
  smtk::attribute::MeshItemPtr item = dynamic_pointer_cast<MeshItem>(this->getObject());
  if (!item)
  {
    return;
  }
  // First - are we allowed to change the number of values?
  const MeshItemDefinition* def = static_cast<const MeshItemDefinition*>(item->definition().get());
  int n = static_cast<int>(item->numberOfValues());
  if (!n && !def->isExtensible())
  {
    return;
  }

  QString strExt = "Meshes";
  if (def->isExtensible() && def->maxNumberOfValues())
  {
    strExt.append(" (Max ").append(QString::number(def->maxNumberOfValues())).append(")");
  }
  else if (!def->isExtensible() && def->numberOfRequiredValues())
  {
    strExt.append(" (Required ").append(QString::number(def->numberOfRequiredValues())).append(")");
  }

  qtMeshItemCombo* editBox = new qtMeshItemCombo(this, this->Widget, strExt);
  editBox->setToolTip("Associate meshes");
  editBox->setModel(new QStandardItemModel());
  editBox->setItemDelegate(new qtCheckableComboItemDelegate(editBox));

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QBoxLayout* editorLayout = new QHBoxLayout;
  editorLayout->setMargin(0);
  editorLayout->setSpacing(3);

  editorLayout->addWidget(editBox);
  this->Internals->EntryLayout->addWidget(editBox, 0, 1);
  editBox->init();
  //  connect(editBox->view()->selectionModel(),
  //    SIGNAL(selectionChanged ( const QItemSelection&, const QItemSelection&)),
  //    this, SLOT(popupViewItemSelected()));

  this->Internals->MeshItemCombo = editBox;
}
