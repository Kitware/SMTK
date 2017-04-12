//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtModelEntityItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/common/UUID.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtCheckItemComboBox.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtSelectionManager.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleValidator>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QSizePolicy>
#include <QStandardItemModel>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>

using namespace smtk::attribute;
using namespace smtk::extension;

class qtModelEntityItemInternals
{
public:

  QPointer<QGridLayout> EntryLayout;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;

  QPointer<QToolButton> LinkSelectionButton;
  QPointer<QToolButton> ClearButton;
  QPointer<qtModelEntityItemCombo> EntityItemCombo;

};

qtModelEntityItem::qtModelEntityItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p, qtBaseView* bview,
   Qt::Orientation enVectorItemOrient)
  :qtItem(dataObj, p, bview)
{
  this->Internals = new qtModelEntityItemInternals;
  this->IsLeafItem = true;
  this->Internals->VectorItemOrient = enVectorItemOrient;
  this->createWidget();
  if (bview)
    {
    bview->uiManager()->onModelEntityItemCreated(this);
    }
}

qtModelEntityItem::~qtModelEntityItem()
{
  delete this->Internals;
}

smtk::attribute::ModelEntityItemPtr qtModelEntityItem::modelEntityItem()
{
  return dynamic_pointer_cast<ModelEntityItem>(this->getObject());
}

void qtModelEntityItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
}

void qtModelEntityItem::createWidget()
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

bool qtModelEntityItem::add(const smtk::model::EntityRef& val)
{
  if (this->modelEntityItem()->appendValue(val))
    {
    emit this->modified();
    smtk::model::EntityRefs addEntityRefs;
    addEntityRefs.insert(val);
    emit this->sendSelectionFromModelEntityToSelectionManager(addEntityRefs, smtk::mesh::MeshSets(),
              smtk::model::DescriptivePhrases(), smtk::extension
                      ::SelectionModifier::SELECTION_ADDITION_UNFILTERED,
                                               smtk::model::StringList());
    return true;
    }
  return false;
}

bool qtModelEntityItem::remove(const smtk::model::EntityRef& val)
{
  auto item = this->modelEntityItem();
  auto idx = item->find(val);
  if(idx < 0)
    {
    return false;
    }

  if(item->isExtensible())
    {
    item->removeValue(idx);
    }
  else
    {
    item->unset(idx);
    }
  emit this->modified();
  smtk::model::EntityRefs removeEntityRefs;
  removeEntityRefs.insert(val);
  emit this->sendSelectionFromModelEntityToSelectionManager(removeEntityRefs, smtk::mesh::MeshSets(),
            smtk::model::DescriptivePhrases(), smtk::extension
                    ::SelectionModifier::SELECTION_SUBTRACTION_UNFILTERED,
                                               smtk::model::StringList());
  return true;
}

void qtModelEntityItem::updateItemData()
{
  this->updateUI();
  this->qtItem::updateItemData();
}

void qtModelEntityItem::addEntityAssociationWidget()
{
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
  if(!item)
    {
    return;
    }
  // First - are we allowed to change the number of values?
  const ModelEntityItemDefinition* def =
    static_cast<const ModelEntityItemDefinition *>(item->definition().get());
  int n = static_cast<int>(item->numberOfValues());
  if (!n && !def->isExtensible())
    {
    return;
    }

  QString strExt = "Entities";
  if (def->isExtensible() && def->maxNumberOfValues())
    strExt.append( " (Max ").append(
      QString::number(def->maxNumberOfValues())).append(")");
  else if(!def->isExtensible() && def->numberOfRequiredValues())
    strExt.append( " (Required ").append(
      QString::number(def->numberOfRequiredValues())).append(")");

  qtModelEntityItemCombo* editBox = new qtModelEntityItemCombo(
    this, this->Widget, strExt);
  editBox->setToolTip("Associate model entities");
  editBox->setModel(new QStandardItemModel());
  editBox->setItemDelegate(
    new qtCheckableComboItemDelegate(editBox));

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QBoxLayout* editorLayout = new QHBoxLayout;
  editorLayout->setMargin(0);
  editorLayout->setSpacing(3);

  // associate button
  this->Internals->LinkSelectionButton = new QToolButton(this->Widget);
  QString iconName(":/icons/attribute/selLinkIn.png");
  this->Internals->LinkSelectionButton->setToolTip("Assoicate With Selected Entities");
  this->Internals->LinkSelectionButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->Internals->LinkSelectionButton->setFixedSize(QSize(16, 16));
  this->Internals->LinkSelectionButton->setIcon(QIcon(iconName));
  this->Internals->LinkSelectionButton->setSizePolicy(sizeFixedPolicy);
  connect(this->Internals->LinkSelectionButton, SIGNAL(clicked()),
    this, SLOT(onRequestEntityAssociation()));
  // clear button
  QToolButton* clearButton = new QToolButton(this->Widget);
  iconName = ":/icons/attribute/clearLinkIn.png";
  clearButton->setToolTip("Clear Entity Associations");
  clearButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  clearButton->setFixedSize(QSize(16, 16));
  clearButton->setIcon(QIcon(iconName));
  clearButton->setSizePolicy(sizeFixedPolicy);
  connect(clearButton, SIGNAL(clicked()),
    this, SLOT(clearEntityAssociations()));
  this->Internals->ClearButton = clearButton;

  editorLayout->addWidget(clearButton);
  editorLayout->addWidget(this->Internals->LinkSelectionButton);

  editorLayout->addWidget(editBox);
  this->Internals->EntryLayout->addLayout(editorLayout, 0, 1);
  editBox->init();
  connect(editBox->view()->selectionModel(),
    SIGNAL(selectionChanged ( const QItemSelection&, const QItemSelection&)),
    this, SLOT(popupViewItemSelected()));

  this->Internals->EntityItemCombo = editBox;
}

void qtModelEntityItem::loadAssociatedEntities()
{
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
  if(!item)
    {
    return;
    }
  this->addEntityAssociationWidget();
}

void qtModelEntityItem::updateUI()
{
  //smtk::attribute::ItemPtr dataObj = this->getObject();
  smtk::attribute::ModelEntityItemPtr dataObj =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
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
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(dataObj);
  const ModelEntityItemDefinition *itemDef =
    dynamic_cast<const ModelEntityItemDefinition*>(dataObj->definition().get());

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

  this->loadAssociatedEntities();

  // we need this layout so that for items with conditionan children,
  // the label will line up at Top-left against the chilren's widgets.
//  QVBoxLayout* vTLlayout = new QVBoxLayout;
//  vTLlayout->setMargin(0);
//  vTLlayout->setSpacing(0);
//  vTLlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
//  vTLlayout->addLayout(labelLayout);
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

void qtModelEntityItem::setOutputOptional(int state)
{
  smtk::attribute::ModelEntityItemPtr item =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
  if(!item)
    {
    return;
    }
  bool enable = state ? true : false;
  if(this->Internals->EntityItemCombo)
    {
    this->Internals->EntityItemCombo->setVisible(enable);
    }
  if(this->Internals->LinkSelectionButton)
    {
    this->Internals->LinkSelectionButton->setVisible(enable);
    this->Internals->ClearButton->setVisible(enable);
    }
  if(enable != this->getObject()->isEnabled())
    {
    this->getObject()->setIsEnabled(enable);
    emit this->modified();
    if(this->baseView())
      {
      this->baseView()->valueChanged(this->getObject());
      }
    }
}

void qtModelEntityItem::associateEntities(
  const smtk::model::EntityRefs& selEntityRefs, bool resetExisting)
{
  smtk::attribute::ModelEntityItemPtr modEntityItem =
    dynamic_pointer_cast<ModelEntityItem>(this->getObject());
  if(!modEntityItem)
    {
    return;
    }
  if(resetExisting)
    modEntityItem->reset();

  std::size_t idx=0;
  for (smtk::model::EntityRefs::const_iterator it = selEntityRefs.begin();
       it != selEntityRefs.end(); ++it)
    {
    bool success = false;
    if(idx < modEntityItem->numberOfValues())
      {
      success = modEntityItem->setValue(idx, *it);
      }

    if(!success)
      {
      success = modEntityItem->appendValue(*it);
      }

    if(!success)
      {
      std::cerr << "ERROR: Unable to set entity to ModelEntityItem: "
                << it->entity().toString() << std::endl;
      }

    ++idx;
    }
  if(this->Internals->EntityItemCombo)
    {
    this->Internals->EntityItemCombo->init();
    }
}

void qtModelEntityItem::clearEntityAssociations()
{
  smtk::attribute::ModelEntityItemPtr modEntityItem = this->modelEntityItem();
  if(!modEntityItem)
    {
    return;
    }
  modEntityItem->reset();

  if(this->Internals->EntityItemCombo)
    {
    this->Internals->EntityItemCombo->init();
    }
  emit this->modified();
  emit this->sendSelectionFromModelEntityToSelectionManager(smtk::model::EntityRefs(), smtk::mesh::MeshSets(),
            smtk::model::DescriptivePhrases(), smtk::extension
                    ::SelectionModifier::SELECTION_REPLACE_UNFILTERED,
                                               smtk::model::StringList());
}

void qtModelEntityItem::onRequestEntityAssociation()
{
  smtk::attribute::ModelEntityItemPtr modEntityItem = this->modelEntityItem();
  if(!modEntityItem)
    {
    return;
    }

  emit this->requestEntityAssociation();
  emit this->modified();
}

void qtModelEntityItem::popupViewItemSelected()
{
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel*>(
    this->Internals->EntityItemCombo->model());
  smtk::common::UUIDs uuids;
  foreach(QModelIndex idx,
          this->Internals->EntityItemCombo->view()->
          selectionModel()->selectedIndexes())
    {
    QStandardItem* item = itemModel->item(idx.row());
    if(!item)
      {
      return;
      }
    QString entid = item->data(Qt::UserRole).toString();
    if(!entid.isEmpty())
      {
      uuids.insert(entid.toStdString());
      }
    }
  emit this->entityListHighlighted(uuids);
}
