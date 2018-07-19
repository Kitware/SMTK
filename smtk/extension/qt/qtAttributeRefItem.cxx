//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAttributeRefItem.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtAttributeView.h"
#include "smtk/extension/qt/qtNewAttributeWidget.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/view/View.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QToolButton>
#include <QVBoxLayout>

using namespace smtk::attribute;
using namespace smtk::extension;

namespace qtAttRefComboInternal
{
void init_Att_Names_and_NEW(QList<QString>& attNames, const RefItemDefinition* itemDef)
{
  smtk::attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
  if (!attDef)
  {
    return;
  }
  attNames.push_back("None");
  std::vector<smtk::attribute::AttributePtr> result;
  ResourcePtr attResource = attDef->resource();
  attResource->findAttributes(attDef, result);
  std::vector<smtk::attribute::AttributePtr>::iterator it;
  for (it = result.begin(); it != result.end(); ++it)
  {
    attNames.push_back((*it)->name().c_str());
  }
  attNames.push_back("New");
}
};

qtAttRefCombo::qtAttRefCombo(smtk::attribute::ItemPtr refitem, QWidget* inParent)
  : QComboBox(inParent)
  , m_RefItem(refitem)
{
  this->setMinimumWidth(80);
}

/*
QSize qtAttRefCombo::sizeHint() const
{
  return QSize(150, this->height());
}
*/

void qtAttRefCombo::showPopup()
{
  QList<QString> attNames;
  if (!m_RefItem.lock())
  {
    this->QComboBox::showPopup();
    return;
  }
  // need to update the list, since it may be changed
  const RefItemDefinition* itemDef =
    dynamic_cast<const RefItemDefinition*>(m_RefItem.lock()->definition().get());
  qtAttRefComboInternal::init_Att_Names_and_NEW(attNames, itemDef);
  this->blockSignals(true);

  RefItemPtr refitem = smtk::dynamic_pointer_cast<RefItem>(m_RefItem.lock());
  int elementIdx = this->property("ElementIndex").toInt();
  int setIndex = 0; // None
  if (refitem->isSet(elementIdx))
  {
    setIndex = attNames.indexOf(refitem->valueAsString(elementIdx).c_str());
  }
  this->clear();
  this->addItems(attNames);
  this->setCurrentIndex(setIndex);
  this->blockSignals(false);

  this->QComboBox::showPopup();
}

class qtAttributeRefItemInternals
{
public:
  QList<qtAttRefCombo*> comboBoxes;
  QPointer<QCheckBox> optionalCheck;
  QPointer<QLabel> theLabel;

  // <elementIdx, qtAtt>
  QMap<int, QPointer<qtAttribute> > RefAtts;
  //  QPointer<qtAttribute> CurrentRefAtt;
  //  QHBoxLayout* RefComboLayout;
  QPointer<QToolButton> EditButton;
  QPointer<QToolButton> CollapseButton;
  bool UserSetAttVisibility;
};

qtItem* qtAttributeRefItem::createItemWidget(const AttributeItemInfo& info)
{
  // So we support this type of item?
  if (info.itemAs<smtk::attribute::ReferenceItem>() == nullptr)
  {
    return nullptr;
  }
  return new qtAttributeRefItem(info);
}

qtAttributeRefItem::qtAttributeRefItem(const AttributeItemInfo& info)
  : qtItem(info)
{
  this->Internals = new qtAttributeRefItemInternals;
  m_isLeafItem = true;
  this->Internals->UserSetAttVisibility = true;
  this->createWidget();
}

qtAttributeRefItem::~qtAttributeRefItem()
{
  foreach (int eleIdx, this->Internals->RefAtts.keys())
  {
    qtAttribute* qa = this->Internals->RefAtts[eleIdx];
    if (qa)
    {
      delete qa;
    }
  }
  delete this->Internals;
}

void qtAttributeRefItem::setLabelVisible(bool visible)
{
  if (this->Internals->theLabel)
  {
    this->Internals->theLabel->setVisible(visible);
    if (visible)
    {
      m_widget->setMinimumWidth(250);
    }
    else
    {
      m_widget->setMinimumWidth(150);
    }
  }
}

void qtAttributeRefItem::setAttributeEditorVisible(bool visible)
{
  this->Internals->EditButton->setVisible(visible);
}

void qtAttributeRefItem::setAttributeWidgetVisible(bool visible)
{
  this->Internals->CollapseButton->setVisible(visible);
  this->setAttributesVisible(visible);
  this->Internals->UserSetAttVisibility = visible;
}

void qtAttributeRefItem::onToggleAttributeWidgetVisibility()
{
  foreach (qtAttribute* qa, this->Internals->RefAtts.values())
  {
    if (qa)
    {
      bool bVisible = qa->widget()->isVisible();
      this->setAttributesVisible(!bVisible);
      this->Internals->CollapseButton->setArrowType(bVisible ? Qt::UpArrow : Qt::DownArrow);
      break;
    }
  }
}

void qtAttributeRefItem::setAttributesVisible(bool visible)
{
  foreach (qtAttribute* qa, this->Internals->RefAtts.values())
  {
    if (qa && qa->widget())
    {
      qa->widget()->setVisible(visible);
    }
  }
  if (this->Internals->UserSetAttVisibility)
  {
    m_itemInfo.baseView()->childrenResized();
  }
}

void qtAttributeRefItem::onLaunchAttributeView()
{
  auto item = m_itemInfo.itemAs<smtk::attribute::ReferenceItem>();
  if (item == nullptr)
  {
    return;
  }
  smtk::view::ViewPtr newAttView(new smtk::view::View("Attribute", "Attribute View"));
  const RefItemDefinition* itemDef =
    dynamic_cast<const RefItemDefinition*>(item->definition().get());
  attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
  newAttView->details().addChild("AttributeTypes").addChild("Type").setContents(attDef->type());

  QDialog attViewDlg;
  attViewDlg.setWindowTitle(
    attDef->label().empty() ? attDef->type().c_str() : attDef->label().c_str());
  QVBoxLayout* layout = new QVBoxLayout(&attViewDlg);
  smtk::extension::ViewInfo vinfo(newAttView, &attViewDlg, m_itemInfo.uiManager());
  qtAttributeView attView(vinfo);
  //layout->addWidget(attView.widget())
  QDialogButtonBox* buttonBox = new QDialogButtonBox(&attViewDlg);
  buttonBox->setStandardButtons(QDialogButtonBox::Ok);
  layout->addWidget(buttonBox);
  attViewDlg.setModal(true);
  QObject::connect(buttonBox, SIGNAL(accepted()), &attViewDlg, SLOT(accept()));
  attViewDlg.exec();

  this->updateItemData();
  foreach (qtAttribute* qa, this->Internals->RefAtts.values())
  {
    this->updateAttWidgetState(qa);
  }
}

void qtAttributeRefItem::createWidget()
{
  auto item = m_itemInfo.itemAs<smtk::attribute::ReferenceItem>();
  if (item == nullptr)
  {
    return;
  }
  this->clearChildItems();
  this->Internals->comboBoxes.clear();
  m_widget = new QFrame(m_itemInfo.parentWidget());
  m_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  std::size_t i, n = item->numberOfValues();
  if (!n)
  {
    return;
  }

  QGridLayout* thisLayout = new QGridLayout(m_widget);
  thisLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  thisLayout->setContentsMargins(0, 0, 0, 0);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  this->Internals->CollapseButton = new QToolButton(m_widget);
  //QString exapndDownName(":/icons/attribute/expand-down.png");
  this->Internals->CollapseButton->setFixedSize(QSize(16, 16));
  this->Internals->CollapseButton->setArrowType(Qt::DownArrow);
  //this->Internals->CollapseButton->setIcon(QIcon(exapndDownName));
  this->Internals->CollapseButton->setToolTip("Toggle referenced attribute widget");
  this->Internals->CollapseButton->setSizePolicy(sizeFixedPolicy);
  connect(this->Internals->CollapseButton, SIGNAL(clicked()), this,
    SLOT(onToggleAttributeWidgetVisibility()));

  this->Internals->EditButton = new QToolButton(m_widget);
  QString resourceName(":/icons/attribute/edit.png");
  this->Internals->EditButton->setFixedSize(QSize(16, 16));
  this->Internals->EditButton->setIcon(QIcon(resourceName));
  this->Internals->EditButton->setSizePolicy(sizeFixedPolicy);
  this->Internals->EditButton->setToolTip("Manage referenced attributes");

  m_widget->setMinimumWidth(250); //combobox width, edit button, collapse button, spacing.

  connect(this->Internals->EditButton, SIGNAL(clicked()), this, SLOT(onLaunchAttributeView()));

  QBoxLayout* labellayout = new QHBoxLayout();
  labellayout->setMargin(0);
  labellayout->setSpacing(3);

  int padding = 0;
  //  int gridCol = 0;
  if (item->isOptional())
  {
    this->Internals->optionalCheck = new QCheckBox(m_widget);
    this->Internals->optionalCheck->setChecked(item->definition()->isEnabledByDefault());
    this->Internals->optionalCheck->setText(" "); // some space
    this->Internals->optionalCheck->setSizePolicy(sizeFixedPolicy);

    if (item->definition()->briefDescription().length())
    {
      this->Internals->optionalCheck->setToolTip(item->definition()->briefDescription().c_str());
    }
    padding = this->Internals->optionalCheck->iconSize().width() + 3; // 3 is for layout spacing

    QObject::connect(this->Internals->optionalCheck, SIGNAL(stateChanged(int)), this,
      SLOT(setOutputOptional(int)));
    labellayout->addWidget(this->Internals->optionalCheck); //, 0, gridCol++);
  }

  QString lText = item->label().c_str();
  this->Internals->theLabel = new QLabel(lText, m_widget);

  this->Internals->theLabel->setFixedWidth(m_itemInfo.baseView()->fixedLabelWidth() - padding);
  this->Internals->theLabel->setSizePolicy(sizeFixedPolicy);
  this->Internals->theLabel->setWordWrap(true);
  this->Internals->theLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  if (item->advanceLevel() > 0)
  {
    this->Internals->theLabel->setFont(m_itemInfo.uiManager()->advancedFont());
  }

  labellayout->addWidget(this->Internals->theLabel);
  labellayout->addWidget(this->Internals->EditButton);
  labellayout->addWidget(this->Internals->CollapseButton);
  thisLayout->addLayout(labellayout, 0, 0);

  //  this->Internals->RefComboLayout->addWidget(this->Internals->EditButton);
  for (i = 0; i < n; i++)
  {
    qtAttRefCombo* combo = new qtAttRefCombo(item, m_widget);
    QVariant vdata(static_cast<int>(i));
    combo->setProperty("ElementIndex", vdata);
    QVBoxLayout* childLayout = new QVBoxLayout;
    childLayout->setContentsMargins(12, 3, 3, 0);

    QVariant vlayoutdata;
    vlayoutdata.setValue(static_cast<void*>(childLayout));
    combo->setProperty("MyLayout", vlayoutdata);
    this->Internals->comboBoxes.push_back(combo);
    combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QObject::connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(onInputValueChanged()),
      Qt::QueuedConnection);

    thisLayout->addWidget(combo, 2 * static_cast<int>(i), 1);
    thisLayout->addLayout(childLayout, 2 * static_cast<int>(i) + 1, 0, 1, 2);
  }

  this->updateItemData();
}

void qtAttributeRefItem::updateAttWidgetState(qtAttribute* qa)
{
  if (qa && qa->widget())
  {
    bool bVisible = (this->Internals->UserSetAttVisibility &&
      this->Internals->CollapseButton->arrowType() == Qt::DownArrow);
    qa->widget()->setVisible(bVisible);
    if (this->Internals->UserSetAttVisibility)
    {
      m_itemInfo.baseView()->childrenResized();
    }
  }
}

void qtAttributeRefItem::updateItemData()
{
  auto item = m_itemInfo.itemAs<RefItem>();
  if (!item)
  {
    return;
  }

  std::size_t n = item->numberOfValues();
  if (!n)
  {
    return;
  }

  const RefItemDefinition* itemDef =
    dynamic_cast<const RefItemDefinition*>(item->definition().get());
  attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
  if (!attDef)
  {
    return;
  }
  QList<QString> attNames;
  qtAttRefComboInternal::init_Att_Names_and_NEW(attNames, itemDef);

  foreach (QComboBox* combo, this->Internals->comboBoxes)
  {
    combo->blockSignals(true);
    combo->clear();
    combo->addItems(attNames);
    int elementIdx = combo->property("ElementIndex").toInt();
    int setIndex = 0; // None
    if (item->isSet(elementIdx))
    {
      setIndex = attNames.indexOf(item->valueAsString(elementIdx).c_str());
      setIndex = setIndex < 0 ? 0 : setIndex;
    }
    combo->setCurrentIndex(setIndex);
    combo->blockSignals(false);
    this->refreshUI(combo);
  }

  if (item->isOptional() && this->Internals->optionalCheck)
  {
    if (this->Internals->optionalCheck->isChecked() == item->isEnabled())
    {
      this->setOutputOptional(this->Internals->optionalCheck->checkState());
    }
    else
    {
      this->Internals->optionalCheck->setChecked(item->isEnabled());
    }
  }
  this->qtItem::updateItemData();
}

void qtAttributeRefItem::setOutputOptional(int state)
{
  bool enable = state ? true : false;
  auto item = m_itemInfo.item();
  this->Internals->EditButton->setVisible(enable);
  this->Internals->CollapseButton->setVisible(enable);
  foreach (QComboBox* combo, this->Internals->comboBoxes)
  {
    combo->setVisible(enable);
  }
  foreach (qtAttribute* qa, this->Internals->RefAtts.values())
  {
    if (qa)
    {
      qa->widget()->setVisible(enable);
    }
  }
  if (enable != item->isEnabled())
  {
    item->setIsEnabled(enable);
    emit this->modified();
    m_itemInfo.baseView()->valueChanged(item);
  }
}

void qtAttributeRefItem::onInputValueChanged()
{
  QComboBox* const comboBox = qobject_cast<QComboBox*>(QObject::sender());
  if (!comboBox)
  {
    return;
  }
  this->refreshUI(comboBox);
}

void qtAttributeRefItem::refreshUI(QComboBox* comboBox)
{
  int curIdx = comboBox->currentIndex();
  int elementIdx = comboBox->property("ElementIndex").toInt();

  auto item = m_itemInfo.itemAs<RefItem>();
  AttributePtr attPtr;
  bool valChanged = true;
  if (curIdx > 0) // index 0 is None
  {
    const RefItemDefinition* itemDef =
      dynamic_cast<const RefItemDefinition*>(item->definition().get());
    attribute::DefinitionPtr attDef = itemDef->attributeDefinition();
    ResourcePtr attResource = attDef->resource();
    if (curIdx == comboBox->count() - 1) // create New attribute
    {
      QList<smtk::attribute::DefinitionPtr> AllDefs;
      m_itemInfo.baseView()->getDefinitions(attDef, AllDefs);
      QList<QString> defTypes;
      QList<QString> defLabels;
      foreach (smtk::attribute::DefinitionPtr aDef, AllDefs)
      {
        if (!aDef->isAbstract())
        {
          std::string txtDef = aDef->label().empty() ? aDef->type() : aDef->label();
          defLabels.push_back(txtDef.c_str());
          defTypes.push_back(aDef->type().c_str());
        }
      }
      if (defTypes.count() > 0)
      {
        std::string attName = attResource->createUniqueName(defTypes[0].toStdString());
        qtNewAttributeWidget attDialog(comboBox);
        attDialog.setBaseWidget(comboBox);
        if (attDialog.showWidget(attName.c_str(), defLabels) == QDialog::Accepted &&
          !attDialog.attributeName().isEmpty())
        {
          int defIndex = defLabels.indexOf(attDialog.attributeType());
          attPtr = attResource->createAttribute(
            attDialog.attributeName().toStdString(), defTypes[defIndex].toStdString());
          comboBox->blockSignals(true);
          comboBox->insertItem(1, attDialog.attributeName());
          comboBox->setCurrentIndex(1);
          comboBox->blockSignals(false);
        }
        else
        {
          int idx = 0;
          // reset combo text
          if (item->isSet(elementIdx))
          {
            attPtr = item->value(elementIdx);
            idx = comboBox->findText(attPtr->name().c_str());
          }
          comboBox->blockSignals(true);
          comboBox->setCurrentIndex(idx < 0 ? 0 : idx);
          comboBox->blockSignals(false);
        }
      }
      else
      {
        QMessageBox::warning(m_widget, tr("Create Attribute"),
          tr("No attribute definition, or all definitions are abstract!"));
      }
    }
    else
    {
      attPtr = attResource->findAttribute(comboBox->currentText().toStdString());
    }

    if (elementIdx >= 0 && static_cast<int>(item->numberOfValues()) > elementIdx &&
      item->isSet(elementIdx) && attPtr == item->value(elementIdx))
    {
      valChanged = false; // nothing to do
    }
    else
    {
      if (attPtr)
      {
        item->setValue(elementIdx, attPtr);
      }
      else if (item->isSet(elementIdx))
      {
        item->unset(elementIdx);
      }
      else
      {
        valChanged = false;
      }
    }
  }
  else if (item->isSet(elementIdx))
  {
    item->unset(elementIdx);
  }
  else
  {
    valChanged = false;
  }

  if (attPtr)
  {
    qtAttribute* currentAtt =
      this->Internals->RefAtts.contains(elementIdx) ? this->Internals->RefAtts[elementIdx] : NULL;
    if (currentAtt && currentAtt->attribute() != attPtr)
    {
      delete currentAtt->widget();
      delete currentAtt;
      currentAtt = NULL;
    }

    if (!currentAtt)
    {
      int currentLen = m_itemInfo.baseView()->fixedLabelWidth();
      int tmpLen = m_itemInfo.uiManager()->getWidthOfAttributeMaxLabel(
        attPtr->definition(), m_itemInfo.uiManager()->advancedFont());
      m_itemInfo.baseView()->setFixedLabelWidth(tmpLen);
      smtk::view::View::Component comp; // currently not used
      currentAtt = new qtAttribute(attPtr, comp, m_widget, m_itemInfo.baseView());
      m_itemInfo.baseView()->setFixedLabelWidth(currentLen);
      if (currentAtt->widget())
      {
        QBoxLayout* mylayout =
          static_cast<QBoxLayout*>(comboBox->property("MyLayout").value<void*>());
        mylayout->addWidget(currentAtt->widget());
      }
      QVariant vrefdata;
      vrefdata.setValue(static_cast<void*>(currentAtt));
      comboBox->setProperty("QtRefAtt", vrefdata);
      this->Internals->RefAtts[elementIdx] = currentAtt;
    }
    this->updateAttWidgetState(currentAtt);
  }
  else // check if we need to remove anything
  {
    QVariant myRefAtt = comboBox->property("QtRefAtt");
    if (myRefAtt.isValid())
    {
      qtAttribute* qa = static_cast<qtAttribute*>(myRefAtt.value<void*>());
      if (qa)
      {
        this->Internals->RefAtts[elementIdx] = NULL;
        delete qa->widget();
        delete qa;
      }
    }
  }

  if (valChanged)
  {
    emit this->modified();
    m_itemInfo.baseView()->valueChanged(item);
  }
}
