//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtMeshSelectionItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/MeshSelectionItemDefinition.h"
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
#include <QButtonGroup>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
class qtMeshSelectionItemInternals
{
public:

  QPointer<QGridLayout> EntryLayout;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;

  QPointer<QToolButton> GrowButton;
  QPointer<QToolButton> GrowPlusButton;
  QPointer<QToolButton> GrowMinusButton;
  QPointer<QToolButton> CancelButton;
  QPointer<QToolButton> AcceptButton;
  QPointer<QButtonGroup> ButtonGroup;

  void uncheckGrowButtons()
  {
    this->ButtonGroup->setExclusive(false);
    this->GrowButton->setChecked(false);
    this->GrowPlusButton->setChecked(false);
    this->GrowMinusButton->setChecked(false);
    this->ButtonGroup->setExclusive(true);
  }

  void modifyOutSelection(const smtk::common::UUID& entid,
                          const std::set<int> vals,
                          MeshSelectionItem::MeshSelectionMode opType)
  {
    if(opType == MeshSelectionItem::RESET)
      {
      m_outSelection[entid] = vals;
      }
    else if(opType == MeshSelectionItem::MERGE)
      {
      m_outSelection[entid].insert(vals.begin(), vals.end());
      }
    else if(opType == MeshSelectionItem::SUBTRACT)
      {
      std::set<int> diffSet;
      std::set_difference(m_outSelection[entid].begin(),
                        m_outSelection[entid].end(),
                        vals.begin(), vals.end(),
                        std::inserter(diffSet, diffSet.end()));
      m_outSelection[entid] = diffSet;
      }
  }

  std::map<smtk::common::UUID, std::set<int> > m_outSelection;
};

//----------------------------------------------------------------------------
qtMeshSelectionItem::qtMeshSelectionItem(
  smtk::attribute::ItemPtr dataObj, QWidget* p, qtBaseView* bview,
   Qt::Orientation enVectorItemOrient) : qtItem(dataObj, p, bview)
{
  this->Internals = new qtMeshSelectionItemInternals;
  this->IsLeafItem = true;
  this->Internals->VectorItemOrient = enVectorItemOrient;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtMeshSelectionItem::~qtMeshSelectionItem()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void qtMeshSelectionItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
}

//----------------------------------------------------------------------------
void qtMeshSelectionItem::createWidget()
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
void qtMeshSelectionItem::updateItemData()
{
  this->updateUI();
  this->qtItem::updateItemData();
}

QToolButton* internal_createToolButton(
  const QString& strIconName, const QString& strToolTip, QWidget* pw,
  QBoxLayout* buttonLayout, QButtonGroup* bgroup, bool checkable,
  qtMeshSelectionItem* meshItem)
{
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QToolButton* retButton = new QToolButton(pw);
  retButton->setToolTip(strToolTip);
  retButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  retButton->setFixedSize(QSize(16, 16));
  retButton->setIcon(QIcon(strIconName));
  retButton->setSizePolicy(sizeFixedPolicy);
  retButton->setCheckable(checkable);

  QObject::connect(retButton, SIGNAL(clicked()),
    meshItem, SLOT(onRequestMeshSelection()));
  buttonLayout->addWidget(retButton);
  bgroup->addButton(retButton);

  return retButton;
}

//----------------------------------------------------------------------------
void qtMeshSelectionItem::addMeshOpButtons()
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
  QButtonGroup* bgroup = new QButtonGroup(this->Widget);
  // grow button
  this->Internals->GrowButton = internal_createToolButton(
    ":/icons/attribute/growcell32.png",
    "Grow Selection on Associated Entities", this->Widget,
    buttonLayout, bgroup, true, this);

  // grow plus button
  this->Internals->GrowPlusButton = internal_createToolButton(
    ":/icons/attribute/growplus32.png",
    "Grow and Append Selection on Selected Entities", this->Widget,
    buttonLayout, bgroup, true, this);

  // grow minus button
  this->Internals->GrowMinusButton = internal_createToolButton(
    ":/icons/attribute/growminus32.png",
    "Grow and Remove Selection on Selected Entities", this->Widget,
    buttonLayout, bgroup, true, this);

  // cancel button
  this->Internals->CancelButton = internal_createToolButton(
    ":/icons/attribute/cancel32.png",
    "Candel Grow Selection Mode", this->Widget,
    buttonLayout, bgroup, false, this);

  this->Internals->AcceptButton = internal_createToolButton(
    ":/icons/attribute/growaccept24.png",
    "Create Face Group With Current Grow Selection", this->Widget,
    buttonLayout, bgroup, false, this);

  this->Internals->EntryLayout->addLayout(buttonLayout, 0, 1);
  this->Internals->ButtonGroup = bgroup;
}

//----------------------------------------------------------------------------
void qtMeshSelectionItem::updateUI()
{
  //smtk::attribute::ItemPtr dataObj = this->getObject();
  smtk::attribute::MeshSelectionItemPtr dataObj =
    dynamic_pointer_cast<MeshSelectionItem>(this->getObject());
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
  smtk::attribute::MeshSelectionItemPtr item =
    dynamic_pointer_cast<MeshSelectionItem>(dataObj);
  const MeshSelectionItemDefinition *itemDef =
    dynamic_cast<const MeshSelectionItemDefinition*>(dataObj->definition().get());

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
void qtMeshSelectionItem::setOutputOptional(int state)
{
  smtk::attribute::MeshSelectionItemPtr item =
    dynamic_pointer_cast<MeshSelectionItem>(this->getObject());
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
void qtMeshSelectionItem::clearSelection()
{
  smtk::attribute::MeshSelectionItemPtr meshSelectionItem =
    dynamic_pointer_cast<MeshSelectionItem>(this->getObject());
  if(!meshSelectionItem)
    {
    return;
    }
  meshSelectionItem->reset();
}

//----------------------------------------------------------------------------
void qtMeshSelectionItem::updateInputSelection(
  const std::map<smtk::common::UUID, std::set<int> >& selectionValues)
{
  smtk::attribute::MeshSelectionItemPtr meshSelectionItem =
    dynamic_pointer_cast<MeshSelectionItem>(this->getObject());
  if(!meshSelectionItem)
    {
    return;
    }

  // clear cached mesh selection list
  this->clearSelection();

  int totalVals = 0;
  smtk::attribute::MeshSelectionItem::const_sel_map_it mapIt;
  for(mapIt = selectionValues.begin(); mapIt != selectionValues.end(); ++mapIt)
    totalVals += mapIt->second.size();

  MeshSelectionItem::MeshSelectionMode opType = meshSelectionItem->meshSelectMode();
  for(mapIt = selectionValues.begin(); mapIt != selectionValues.end(); ++mapIt)
    {
    switch(opType)
      {
      case MeshSelectionItem::ACCEPT:
        meshSelectionItem->setValues(mapIt->first, mapIt->second);
        break;
      case MeshSelectionItem::RESET:
      case MeshSelectionItem::MERGE:
      case MeshSelectionItem::SUBTRACT:
        if(meshSelectionItem->isCtrlKeyDown() || totalVals > 1)
          {
          this->Internals->modifyOutSelection(mapIt->first, mapIt->second, opType);
          meshSelectionItem->setValues(
            mapIt->first, this->Internals->m_outSelection[mapIt->first]);
          }
        else if(totalVals == 1)
          meshSelectionItem->setValues(mapIt->first, mapIt->second);
        break;
      case MeshSelectionItem::NONE:
       break;
      default:
        std::cerr << "ERROR: Unrecognized MeshUpdateMode: "
                  << opType << std::endl;
        break;
      }
    }

  if(opType == MeshSelectionItem::ACCEPT)
    this->Internals->m_outSelection.clear();
  else if(opType == MeshSelectionItem::ACCEPT)
    {
    this->Internals->uncheckGrowButtons();
    this->Internals->m_outSelection.clear();
    }
  else if(opType == MeshSelectionItem::NONE)
    {
    this->Internals->uncheckGrowButtons();
    this->Internals->m_outSelection.clear();
    meshSelectionItem->reset();
    }
}

//----------------------------------------------------------------------------
smtk::attribute::ModelEntityItemPtr qtMeshSelectionItem::refModelEntityItem()
{
  smtk::attribute::MeshSelectionItemPtr meshSelectionItem =
    dynamic_pointer_cast<MeshSelectionItem>(this->getObject());
  if(!meshSelectionItem)
    {
    return smtk::attribute::ModelEntityItemPtr();
    }
  const MeshSelectionItemDefinition *itemDef =
    dynamic_cast<const MeshSelectionItemDefinition*>(meshSelectionItem->definition().get());
  smtk::attribute::AttributePtr att = meshSelectionItem->attribute();
  return att->findModelEntity(itemDef->refModelEntityName());
}
//----------------------------------------------------------------------------
void qtMeshSelectionItem::setUsingCtrlKey(bool val)
{
  smtk::attribute::MeshSelectionItemPtr meshSelectionItem =
    dynamic_pointer_cast<MeshSelectionItem>(this->getObject());
  if(!meshSelectionItem)
    {
    return;
    }
  meshSelectionItem->setCtrlKeyDown(val);
}
//----------------------------------------------------------------------------
bool qtMeshSelectionItem::usingCtrlKey()
{
  smtk::attribute::MeshSelectionItemPtr meshSelectionItem =
    dynamic_pointer_cast<MeshSelectionItem>(this->getObject());
  if(!meshSelectionItem)
    {
    return false;
    }
  return meshSelectionItem->isCtrlKeyDown();
}

//----------------------------------------------------------------------------
void qtMeshSelectionItem::onRequestMeshSelection()
{
  QToolButton* const cButton = qobject_cast<QToolButton*>(
    QObject::sender());
  if(!cButton)
    {
    return;
    }
  smtk::attribute::MeshSelectionItemPtr meshSelectionItem =
    dynamic_pointer_cast<MeshSelectionItem>(this->getObject());
  if(!meshSelectionItem)
    {
    return;
    }

  this->setUsingCtrlKey(false);

  MeshSelectionItem::MeshSelectionMode selType;
  if(cButton == this->Internals->AcceptButton)
    selType = MeshSelectionItem::ACCEPT;
  else if(cButton == this->Internals->GrowButton)
    selType = MeshSelectionItem::RESET;
  else if(cButton == this->Internals->GrowPlusButton)
    selType = MeshSelectionItem::MERGE;
  else if(cButton == this->Internals->GrowMinusButton)
    selType = MeshSelectionItem::SUBTRACT;
  else if(cButton == this->Internals->CancelButton)
    selType = MeshSelectionItem::NONE;
  else
    {
    std::cerr << "ERROR: Unrecognized button click "
              << cButton->objectName().toStdString() << std::endl;
    return;
    }

  meshSelectionItem->setMeshSelectMode(selType);
  if(selType == MeshSelectionItem::ACCEPT || selType == MeshSelectionItem::NONE)
    this->Internals->uncheckGrowButtons();

  smtk::attribute::ModelEntityItem::Ptr modelEntities =
    this->refModelEntityItem();
  if(modelEntities)
    emit this->requestMeshSelection(modelEntities);
}

//----------------------------------------------------------------------------
void qtMeshSelectionItem::syncWithCachedSelection(
  const smtk::attribute::MeshSelectionItemPtr& resultSelectionItem,
  std::map<smtk::common::UUID, std::set<int> > &outSelectionValues)
{
  smtk::attribute::MeshSelectionItemPtr meshSelectionItem =
    dynamic_pointer_cast<MeshSelectionItem>(this->getObject());
  if(!meshSelectionItem)
    {
    return;
    }

  smtk::attribute::MeshSelectionItem::const_sel_map_it mapIt;
  MeshSelectionItem::MeshSelectionMode opType = meshSelectionItem->meshSelectMode();
  if(opType == MeshSelectionItem::RESET ||
    opType == MeshSelectionItem::ACCEPT ||
    opType == MeshSelectionItem::NONE)
    this->Internals->m_outSelection.clear();

  for(mapIt = resultSelectionItem->begin(); mapIt != resultSelectionItem->end(); ++mapIt)
    {
    switch(opType)
      {
      case MeshSelectionItem::ACCEPT:
      case MeshSelectionItem::NONE:
        break;
      case MeshSelectionItem::RESET:
        this->Internals->modifyOutSelection(mapIt->first, mapIt->second, opType);
        break;
      case MeshSelectionItem::MERGE:
      case MeshSelectionItem::SUBTRACT:
        if(meshSelectionItem->numberOfValues() == 1)
        // this result is from grow, so we need to update the cached selection with it
          {
          this->Internals->modifyOutSelection(mapIt->first, mapIt->second, opType);
          }
        // else, // the cached selection should already be updated
        break;
      default:
        std::cerr << "ERROR: Unrecognized MeshUpdateMode: "
                  << opType << std::endl;
        break;
      }
    }
  // copy cached selection out
  if(opType == MeshSelectionItem::RESET ||
     opType == MeshSelectionItem::MERGE ||
     opType == MeshSelectionItem::SUBTRACT )
    {
    for(mapIt = this->Internals->m_outSelection.begin();
      mapIt != this->Internals->m_outSelection.end(); ++mapIt)
        outSelectionValues[mapIt->first] =
          this->Internals->m_outSelection[mapIt->first];
    }
}
