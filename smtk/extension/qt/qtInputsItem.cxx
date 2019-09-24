//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtInputsItem.h"

#include "smtk/attribute/Definition.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtDiscreteValueEditor.h"
#include "smtk/extension/qt/qtDoubleLineEdit.h"
#include "smtk/extension/qt/qtOverlay.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDoubleValidator>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPointer>
#include <QSizePolicy>
#include <QSpinBox>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/ValueItemTemplate.h"

#include "smtk/common/CompilerInformation.h"

#include <math.h>

#if defined(SMTK_MSVC) && _MSC_VER <= 1500
#include <float.h>
double nextafter(double x, double y)
{
  return _nextafter(x, y);
}
#endif

using namespace smtk::attribute;
using namespace smtk::extension;

qtDoubleValidator::qtDoubleValidator(
  qtInputsItem* item, int elementIndex, QLineEdit* lineEdit, QObject* inParent)
  : QDoubleValidator(inParent)
  , m_item(item)
  , m_elementIndex(elementIndex)
  , m_lineWidget(lineEdit)
{
}

void qtDoubleValidator::fixup(QString& input) const
{
  auto item = m_item->itemAs<ValueItem>();
  if (!item)
  {
    return;
  }
  if (input.length() == 0)
  {
    m_item->unsetValue(m_elementIndex);
    m_item->uiManager()->setWidgetColorToInvalid(m_lineWidget);
    return;
  }

  auto dDef = item->definitionAs<DoubleItemDefinition>();
  if (item->isSet(m_elementIndex))
  {
    input = item->valueAsString(m_elementIndex).c_str();
  }
  else if (dDef->hasDefault())
  {
    input = QString::number(dDef->defaultValue(m_elementIndex));
  }
  else
  {
    m_item->uiManager()->setWidgetColorToInvalid(m_lineWidget);
  }
}

qtIntValidator::qtIntValidator(
  qtInputsItem* item, int elementIndex, QLineEdit* lineEdit, QObject* inParent)
  : QIntValidator(inParent)
  , m_item(item)
  , m_elementIndex(elementIndex)
  , m_lineWidget(lineEdit)
{
}

void qtIntValidator::fixup(QString& input) const
{
  auto item = m_item->itemAs<ValueItem>();
  if (!item)
  {
    return;
  }
  if (input.length() == 0)
  {
    m_item->unsetValue(m_elementIndex);
    m_item->uiManager()->setWidgetColorToInvalid(m_lineWidget);
    return;
  }

  auto dDef = item->definitionAs<IntItemDefinition>();
  if (item->isSet(m_elementIndex))
  {
    input = item->valueAsString(m_elementIndex).c_str();
  }
  else if (dDef->hasDefault())
  {
    input = QString::number(dDef->defaultValue(m_elementIndex));
  }
  else
  {
    m_item->uiManager()->setWidgetColorToInvalid(m_lineWidget);
  }
}

class qtInputsItemInternals
{
public:
  QPointer<QGridLayout> EntryLayout;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;

  // for discrete items that with potential child widget
  // <Enum-Combo, child-layout >
  QMap<QWidget*, QPointer<QLayout> > ChildrenMap;

  // for extensible items
  QMap<QToolButton*, QPair<QPointer<QLayout>, QPointer<QWidget> > > ExtensibleMap;
  QList<QToolButton*> MinusButtonIndices;
  QPointer<QToolButton> AddItemButton;
  QList<QPointer<qtDiscreteValueEditor> > DiscreteEditors;
  int m_editPrecision;
};

qtItem* qtInputsItem::createItemWidget(const qtAttributeItemInfo& info)
{
  // So we support this type of item?
  if (info.itemAs<smtk::attribute::ValueItem>() == nullptr)
  {
    return nullptr;
  }
  return new qtInputsItem(info);
}

qtInputsItem::qtInputsItem(const qtAttributeItemInfo& info)
  : qtItem(info)
{
  this->Internals = new qtInputsItemInternals;
  m_isLeafItem = true;
  this->Internals->VectorItemOrient = Qt::Horizontal;
  this->Internals->m_editPrecision = 0; // Use full precision by default
  // See if we are suppose to override it
  m_itemInfo.component().attributeAsInt("EditPrecision", this->Internals->m_editPrecision);

  this->createWidget();
}

qtInputsItem::~qtInputsItem()
{
  delete this->Internals;
}

void qtInputsItem::unsetValue(int elementIndex)
{
  auto item = m_itemInfo.itemAs<ValueItem>();
  if (item->isSet(elementIndex))
  {
    item->unset(elementIndex);
    emit modified();
    auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
    if (iview)
    {
      iview->valueChanged(item);
    }
  }
}

bool qtInputsItem::setDiscreteValue(int elementIndex, int discreteValIndex)
{
  auto item = m_itemInfo.itemAs<ValueItem>();
  auto oldIndex = item->discreteIndex(elementIndex);
  // Would we actually change the value?
  if (item->isSet(elementIndex) && (oldIndex == discreteValIndex))
  {
    // Return true to indicate that the input item is in a valid state and can be processed
    // This is needed for updating active children in the discrete editor
    return true;
  }
  else if (item->setDiscreteIndex(elementIndex, discreteValIndex))
  {
    emit this->modified();
    auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
    if (iview)
    {
      iview->valueChanged(item);
    }
    return true;
  }
  return false;
}

void qtInputsItem::forceUpdate()
{
  auto item = m_itemInfo.itemAs<ValueItem>();
  emit this->modified();
  auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
  if (iview)
  {
    iview->valueChanged(item);
  }
}

void qtInputsItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
}

void qtInputsItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = m_itemInfo.item();
  auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
  if (iview && !iview->displayItem(dataObj))
  {
    return;
  }

  this->clearChildWidgets();
  this->updateUI();
}

void qtInputsItem::updateItemData()
{
  auto valItem = m_itemInfo.itemAs<ValueItem>();
  if (valItem == nullptr)
  {
    return;
  }

  if (valItem->isDiscrete())
  {
    // Ok we are dealing with discrete editors
    foreach (QWidget* cwidget, this->Internals->ChildrenMap.keys())
    {
      qtDiscreteValueEditor* editor = qobject_cast<qtDiscreteValueEditor*>(cwidget);
      if (editor != nullptr)
      {
        editor->updateItemData();
      }
    }
    this->qtItem::updateItemData();
    return;
  }

  auto doubleItem = m_itemInfo.itemAs<DoubleItem>();
  if (doubleItem != nullptr)
  {
    foreach (QWidget* cwidget, this->Internals->ChildrenMap.keys())
    {
      this->updateDoubleItemData(cwidget, doubleItem);
    }
    this->qtItem::updateItemData();
    return;
  }
  auto intItem = m_itemInfo.itemAs<IntItem>();
  if (intItem != nullptr)
  {
    foreach (QWidget* cwidget, this->Internals->ChildrenMap.keys())
    {
      this->updateIntItemData(cwidget, intItem);
    }
    this->qtItem::updateItemData();
    return;
  }

  auto stringItem = m_itemInfo.itemAs<StringItem>();
  if (stringItem != nullptr)
  {
    foreach (QWidget* cwidget, this->Internals->ChildrenMap.keys())
    {
      this->updateStringItemData(cwidget, stringItem);
    }
    this->qtItem::updateItemData();
    return;
  }
  this->qtItem::updateItemData();
}

void qtInputsItem::updateDoubleItemData(
  QWidget* iwidget, const smtk::attribute::DoubleItemPtr& ditem)
{
  int elementIdx = iwidget->property("ElementIndex").toInt();
  QLineEdit* lineEdit = qobject_cast<QLineEdit*>(iwidget);
  bool isValid = ditem->isSet(elementIdx);
  bool isDefault = ditem->isUsingDefault(elementIdx);
  if (lineEdit != nullptr)
  {
    QString ival;
    if (isValid)
    {
      if (this->Internals->m_editPrecision > 0)
      {
        ival = QString::number(ditem->value(elementIdx), 'f', this->Internals->m_editPrecision);
      }
      else
      {
        ival = ditem->valueAsString(elementIdx).c_str();
      }
      if (lineEdit->text() != ival)
      {
        lineEdit->blockSignals(true);
        lineEdit->setText(ival);
        lineEdit->blockSignals(false);
      }
    }
    else if (lineEdit->text() != "")
    {
      lineEdit->blockSignals(true);
      lineEdit->setText("");
      lineEdit->blockSignals(false);
    }
  }
  else
  {
    QDoubleSpinBox* spinbox = qobject_cast<QDoubleSpinBox*>(iwidget);
    if (spinbox == nullptr)
    {
      return; // Can't figure out the widget being used
    }
    if (isValid && (ditem->value(elementIdx) != spinbox->value()))
    {
      spinbox->blockSignals(true);
      spinbox->setValue(ditem->value(elementIdx));
      spinbox->blockSignals(false);
    }
  }
  qtUIManager* uimanager = this->uiManager();
  isDefault ? uimanager->setWidgetColorToDefault(iwidget)
            : (isValid ? uimanager->setWidgetColorToNormal(iwidget)
                       : uimanager->setWidgetColorToInvalid(iwidget));
}

void qtInputsItem::updateIntItemData(QWidget* iwidget, const smtk::attribute::IntItemPtr& iitem)
{
  int elementIdx = iwidget->property("ElementIndex").toInt();
  QLineEdit* lineEdit = qobject_cast<QLineEdit*>(iwidget);
  bool isValid = iitem->isSet(elementIdx);
  bool isDefault = iitem->isUsingDefault(elementIdx);
  if (lineEdit != nullptr)
  {
    if (isValid && (iitem->valueAsString(elementIdx).c_str() != lineEdit->text()))
    {
      lineEdit->blockSignals(true);
      lineEdit->setText(iitem->valueAsString(elementIdx).c_str());
      lineEdit->blockSignals(false);
    }
    else if ((!isValid) && (lineEdit->text() != ""))
    {
      lineEdit->blockSignals(true);
      lineEdit->setText("");
      lineEdit->blockSignals(false);
    }
  }
  else
  {
    QSpinBox* spinbox = qobject_cast<QSpinBox*>(iwidget);
    if (spinbox == nullptr)
    {
      return; // Can't figure out the widget being used
    }
    if (isValid && (iitem->value(elementIdx) != spinbox->value()))
    {
      spinbox->blockSignals(true);
      spinbox->setValue(iitem->value(elementIdx));
      spinbox->blockSignals(false);
    }
  }
  qtUIManager* uimanager = this->uiManager();
  isDefault ? uimanager->setWidgetColorToDefault(iwidget)
            : (isValid ? uimanager->setWidgetColorToNormal(iwidget)
                       : uimanager->setWidgetColorToInvalid(iwidget));
}

void qtInputsItem::updateStringItemData(
  QWidget* iwidget, const smtk::attribute::StringItemPtr& sitem)
{
  int elementIdx = iwidget->property("ElementIndex").toInt();
  QLineEdit* lineEdit = qobject_cast<QLineEdit*>(iwidget);
  bool isValid = sitem->isSet(elementIdx);
  bool isDefault = sitem->isUsingDefault(elementIdx);
  if (lineEdit != nullptr)
  {
    if (isValid && (sitem->valueAsString(elementIdx).c_str() != lineEdit->text()))
    {
      lineEdit->blockSignals(true);
      lineEdit->setText(sitem->valueAsString(elementIdx).c_str());
      lineEdit->blockSignals(false);
    }
    else if ((!isValid) && (lineEdit->text() != ""))
    {
      lineEdit->blockSignals(true);
      lineEdit->setText("");
      lineEdit->blockSignals(false);
    }
  }
  else
  {
    QTextEdit* textEdit = qobject_cast<QTextEdit*>(iwidget);
    if (textEdit == nullptr)
    {
      return; // Can't figure out the widget being used
    }
    if (isValid && (sitem->value(elementIdx).c_str() != textEdit->toPlainText()))
    {
      textEdit->blockSignals(true);
      textEdit->setText(sitem->value(elementIdx).c_str());
      textEdit->blockSignals(false);
    }
    else if ((!isValid) && (textEdit->toPlainText() != ""))
    {
      textEdit->blockSignals(true);
      textEdit->setText("");
      textEdit->blockSignals(false);
    }
  }
  qtUIManager* uimanager = this->uiManager();
  isDefault ? uimanager->setWidgetColorToDefault(iwidget)
            : (isValid ? uimanager->setWidgetColorToNormal(iwidget)
                       : uimanager->setWidgetColorToInvalid(iwidget));
}

void qtInputsItem::addInputEditor(int i)
{
  auto item = m_itemInfo.itemAs<ValueItem>();
  if (!item)
  {
    return;
  }

  int n = static_cast<int>(item->numberOfValues());
  if (!n)
  {
    return;
  }
  QBoxLayout* childLayout = NULL;
  if (item->isDiscrete())
  {
    childLayout = new QVBoxLayout;
    childLayout->setContentsMargins(12, 3, 3, 0);
    childLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  }

  QWidget* editBox = this->createInputWidget(i, childLayout);
  if (!editBox)
  {
    return;
  }
  auto itemDef = item->definitionAs<ValueItemDefinition>();
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QBoxLayout* editorLayout = new QHBoxLayout;
  editorLayout->setMargin(0);
  editorLayout->setSpacing(3);
  if (item->isExtensible())
  {
    QToolButton* minusButton = new QToolButton(m_widget);
    QString iconName(":/icons/attribute/minus.png");
    minusButton->setFixedSize(QSize(12, 12));
    minusButton->setIcon(QIcon(iconName));
    minusButton->setSizePolicy(sizeFixedPolicy);
    minusButton->setToolTip("Remove value");
    editorLayout->addWidget(minusButton);
    connect(minusButton, SIGNAL(clicked()), this, SLOT(onRemoveValue()));
    QPair<QPointer<QLayout>, QPointer<QWidget> > pair;
    pair.first = editorLayout;
    pair.second = editBox;
    this->Internals->ExtensibleMap[minusButton] = pair;
    this->Internals->MinusButtonIndices.push_back(minusButton);
  }

  if (n != 1 && itemDef->hasValueLabels())
  {
    std::string componentLabel = itemDef->valueLabel(i);
    if (!componentLabel.empty())
    {
      // acbauer -- this should probably be improved to look nicer
      QString labelText = componentLabel.c_str();
      QLabel* label = new QLabel(labelText, editBox);
      label->setSizePolicy(sizeFixedPolicy);
      editorLayout->addWidget(label);
    }
  }
  editorLayout->addWidget(editBox);

  // always going vertical for discrete and extensible items
  if (this->Internals->VectorItemOrient == Qt::Vertical || item->isDiscrete() ||
    item->isExtensible())
  {
    int row = item->isDiscrete() ? 2 * i : i;
    // The "Add New Value" button is in first row, so take that into account
    row = item->isExtensible() ? row + 1 : row;
    this->Internals->EntryLayout->addLayout(editorLayout, row, 1);
    // there could be conditional children, so we need another layout
    // so that the combobox will stay TOP-left when there are multiple
    // combo boxes.
    if (item->isDiscrete() && childLayout)
    {
      this->Internals->EntryLayout->addLayout(childLayout, row + 1, 0, 1, 2);
    }
  }
  else // going horizontal
  {
    this->Internals->EntryLayout->addLayout(editorLayout, 0, i + 1);
  }

  this->Internals->ChildrenMap[editBox] = childLayout;
  this->updateExtensibleState();
}

void qtInputsItem::loadInputValues()
{
  smtk::attribute::ValueItemPtr item = m_itemInfo.itemAs<ValueItem>();
  if (!item)
  {
    return;
  }

  int n = static_cast<int>(item->numberOfValues());
  if (!n && !item->isExtensible())
  {
    return;
  }

  if (item->isExtensible())
  {
    if (!this->Internals->AddItemButton)
    {
      QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      this->Internals->AddItemButton = new QToolButton(m_widget);
      QString iconName(":/icons/attribute/plus.png");
      this->Internals->AddItemButton->setText("Add New Value");
      this->Internals->AddItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

      //      this->Internals->AddItemButton->setFixedSize(QSize(12, 12));
      this->Internals->AddItemButton->setIcon(QIcon(iconName));
      this->Internals->AddItemButton->setSizePolicy(sizeFixedPolicy);
      connect(this->Internals->AddItemButton, SIGNAL(clicked()), this, SLOT(onAddNewValue()));
      this->Internals->EntryLayout->addWidget(this->Internals->AddItemButton, 0, 1);
    }
  }

  for (int i = 0; i < n; i++)
  {
    this->addInputEditor(i);
  }
}

void qtInputsItem::updateUI()
{
  smtk::attribute::ValueItemPtr dataObj = m_itemInfo.itemAs<ValueItem>();
  auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
  if (iview && !iview->displayItem(dataObj))
  {
    return;
  }

  m_widget = new QFrame(this->parentWidget());
  if (this->isReadOnly())
  {
    m_widget->setEnabled(false);
  }
  this->Internals->EntryLayout = new QGridLayout(m_widget);
  this->Internals->EntryLayout->setMargin(0);
  this->Internals->EntryLayout->setSpacing(0);
  this->Internals->EntryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if (dataObj->isOptional())
  {
    QCheckBox* optionalCheck = new QCheckBox(this->parentWidget());
    optionalCheck->setChecked(dataObj->isEnabled());
    optionalCheck->setText(" ");
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = optionalCheck->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)), this, SLOT(setOutputOptional(int)));
    labelLayout->addWidget(optionalCheck);
  }
  auto itemDef = dataObj->definitionAs<ValueItemDefinition>();

  QString labelText;
  if (!dataObj->label().empty())
  {
    labelText = dataObj->label().c_str();
  }
  else
  {
    labelText = dataObj->name().c_str();
  }
  QLabel* label = new QLabel(labelText, m_widget);
  label->setSizePolicy(sizeFixedPolicy);
  if (iview)
  {
    label->setFixedWidth(iview->fixedLabelWidth() - padding);
  }
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  //  qtOverlayFilter *filter = new qtOverlayFilter(this);
  //  label->installEventFilter(filter);

  // add in BriefDescription as tooltip if available
  const std::string strBriefDescription = itemDef->briefDescription();
  if (!strBriefDescription.empty())
  {
    label->setToolTip(strBriefDescription.c_str());
  }

  if (!itemDef->units().empty())
  {
    // Are we using a spin box?  If so we don't need to add units
    std::string option = "LineEdit"; // defualt behavior
    m_itemInfo.component().attribute("Option", option);

    if (option == "LineEdit")
    {
      QString unitText = label->text();
      unitText.append(" (").append(itemDef->units().c_str()).append(")");
      label->setText(unitText);
    }
  }
  if (itemDef->advanceLevel() && m_itemInfo.baseView())
  {
    label->setFont(m_itemInfo.uiManager()->advancedFont());
  }
  labelLayout->addWidget(label);
  this->Internals->theLabel = label;

  this->loadInputValues();

  // we need this layout so that for items with conditionan children,
  // the label will line up at Top-left against the chilren's widgets.
  //  QVBoxLayout* vTLlayout = new QVBoxLayout;
  //  vTLlayout->setMargin(0);
  //  vTLlayout->setSpacing(0);
  //  vTLlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  //  vTLlayout->addLayout(labelLayout);
  this->Internals->EntryLayout->addLayout(labelLayout, 0, 0);
  //  layout->addWidget(this->Internals->EntryFrame, 0, 1);
  if (this->parentWidget() && this->parentWidget()->layout())
  {
    this->parentWidget()->layout()->addWidget(m_widget);
  }
  if (dataObj->isOptional())
  {
    this->setOutputOptional(dataObj->isEnabled() ? 1 : 0);
  }
}

void qtInputsItem::setOutputOptional(int state)
{
  smtk::attribute::ValueItemPtr item = m_itemInfo.itemAs<ValueItem>();
  if (!item)
  {
    return;
  }
  bool enable = state ? true : false;
  if (item->isExtensible())
  {
    if (this->Internals->AddItemButton)
    {
      this->Internals->AddItemButton->setVisible(enable);
    }
    foreach (QToolButton* tButton, this->Internals->ExtensibleMap.keys())
    {
      tButton->setVisible(enable);
    }
  }

  foreach (QWidget* cwidget, this->Internals->ChildrenMap.keys())
  {
    QLayout* childLayout = this->Internals->ChildrenMap.value(cwidget);
    if (childLayout)
    {
      for (int i = 0; i < childLayout->count(); ++i)
        childLayout->itemAt(i)->widget()->setVisible(enable);
    }
    cwidget->setVisible(enable);
  }

  //  this->Internals->EntryFrame->setEnabled(enable);
  if (enable != item->isEnabled())
  {
    item->setIsEnabled(enable);
    auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
    if (iview)
    {
      iview->valueChanged(item);
    }
    emit this->modified();
  }
}

void qtInputsItem::onAddNewValue()
{
  smtk::attribute::ValueItemPtr item = m_itemInfo.itemAs<ValueItem>();
  if (!item)
  {
    return;
  }
  if (item->setNumberOfValues(item->numberOfValues() + 1))
  {
    //    QBoxLayout* entryLayout = qobject_cast<QBoxLayout*>(
    //      this->Internals->EntryFrame->layout());
    this->addInputEditor(static_cast<int>(item->numberOfValues()) - 1);
  }
  emit this->modified();
}

void qtInputsItem::onRemoveValue()
{
  QToolButton* const minusButton = qobject_cast<QToolButton*>(QObject::sender());
  if (!minusButton || !this->Internals->ExtensibleMap.contains(minusButton))
  {
    return;
  }

  int gIdx = this->Internals->MinusButtonIndices.indexOf(
    minusButton); //minusButton->property("SubgroupIndex").toInt();
  smtk::attribute::ValueItemPtr item = m_itemInfo.itemAs<ValueItem>();
  if (!item || gIdx < 0 || gIdx >= static_cast<int>(item->numberOfValues()))
  {
    return;
  }

  bool status = false;
  switch (item->type())
  {
    case smtk::attribute::Item::DoubleType:
    {
      status = dynamic_pointer_cast<DoubleItem>(item)->removeValue(gIdx);
      break;
    }
    case smtk::attribute::Item::IntType:
    {
      status = dynamic_pointer_cast<IntItem>(item)->removeValue(gIdx);
      break;
    }
    case smtk::attribute::Item::StringType:
    {
      status = dynamic_pointer_cast<StringItem>(item)->removeValue(gIdx);
      break;
    }
    default:
      //m_errorStatus << "Error: Unsupported Item Type: " <<
      // smtk::attribute::Item::type2String(item->type()) << "\n";
      break;
  }

  // Were we able to remove the value?
  if (!status)
  {
    // Nothing to update since nothing changed
    return;
  }
  this->clearChildWidgets();
  this->loadInputValues();
  emit this->modified();
}

void qtInputsItem::updateExtensibleState()
{
  smtk::attribute::ValueItemPtr item = m_itemInfo.itemAs<ValueItem>();
  if (!item || !item->isExtensible())
  {
    return;
  }
  bool maxReached =
    (item->maxNumberOfValues() > 0) && (item->maxNumberOfValues() == item->numberOfValues());
  this->Internals->AddItemButton->setEnabled(!maxReached);

  bool minReached = (item->numberOfRequiredValues() > 0) &&
    (item->numberOfRequiredValues() == item->numberOfValues());
  foreach (QToolButton* tButton, this->Internals->ExtensibleMap.keys())
  {
    tButton->setEnabled(!minReached);
  }
}

void qtInputsItem::clearChildWidgets()
{
  smtk::attribute::ValueItemPtr item = m_itemInfo.itemAs<ValueItem>();
  if (!item)
  {
    return;
  }

  if (item->isExtensible())
  {
    //clear mapping
    foreach (QToolButton* tButton, this->Internals->ExtensibleMap.keys())
    {
      // will delete later from this->Internals->ChildrenMap
      //      delete this->Internals->ExtensibleMap.value(tButton).second;
      delete this->Internals->ExtensibleMap.value(tButton).first;
      delete tButton;
    }
    this->Internals->ExtensibleMap.clear();
    this->Internals->MinusButtonIndices.clear();
  }

  foreach (QWidget* cwidget, this->Internals->ChildrenMap.keys())
  {
    QLayout* childLayout = this->Internals->ChildrenMap.value(cwidget);
    if (childLayout)
    {
      QLayoutItem* child;
      while ((child = childLayout->takeAt(0)) != 0)
      {
        delete child;
      }
      delete childLayout;
    }
    delete cwidget;
  }
  this->Internals->ChildrenMap.clear();
  this->Internals->DiscreteEditors.clear();
}

QWidget* qtInputsItem::createInputWidget(int elementIdx, QLayout* childLayout)
{
  smtk::attribute::ValueItemPtr item = m_itemInfo.itemAs<ValueItem>();
  if (!item)
  {
    return NULL;
  }

  if (item->allowsExpressions())
  {
    return this->createExpressionRefWidget(elementIdx);
  }

  if (item->isDiscrete())
  {
    auto editor = new qtDiscreteValueEditor(this, elementIdx, childLayout);
    QObject::connect(editor, SIGNAL(widgetSizeChanged()), this, SIGNAL(widgetSizeChanged()));
    // editor->setUseSelectionManager(m_useSelectionManager);
    this->Internals->DiscreteEditors.append(editor);
    return editor;
  }
  return this->createEditBox(elementIdx, m_widget);
}

QWidget* qtInputsItem::createExpressionRefWidget(int elementIdx)
{
  smtk::attribute::ValueItemPtr inputitem = m_itemInfo.itemAs<ValueItem>();
  if (!inputitem)
  {
    return NULL;
  }

  QFrame* checkFrame = new QFrame(m_widget);
  QHBoxLayout* mainlayout = new QHBoxLayout(checkFrame);

  QToolButton* funCheck = new QToolButton(checkFrame);
  funCheck->setCheckable(true);
  QString resourceName(":/icons/attribute/function.png");
  funCheck->setIconSize(QSize(13, 13));
  funCheck->setIcon(QIcon(resourceName));
  funCheck->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  funCheck->setToolTip("Switch between a constant value or function instance");
  QVariant vdata(elementIdx);
  funCheck->setProperty("ElementIndex", vdata);

  // create combobox for expression reference
  QComboBox* combo = new QComboBox(checkFrame);
  combo->setProperty("ElementIndex", vdata);
  QObject::connect(combo, SIGNAL(currentIndexChanged(int)), this,
    SLOT(onExpressionReferenceChanged()), Qt::QueuedConnection);

  // check if there are attributes already created, if not
  // disable the function checkbox
  auto valItemDef = inputitem->definitionAs<ValueItemDefinition>();
  smtk::attribute::DefinitionPtr attDef = valItemDef->expressionDefinition();
  std::vector<smtk::attribute::AttributePtr> result;
  if (attDef)
  {
    ResourcePtr lAttResource = attDef->resource();
    lAttResource->findAttributes(attDef, result);
  }
  funCheck->setEnabled(result.size() > 0);

  // create line edit for expression which is a const value
  QWidget* valeditor = this->createEditBox(elementIdx, checkFrame);

  mainlayout->addWidget(funCheck);
  mainlayout->addWidget(valeditor);
  combo->setVisible(0);
  mainlayout->addWidget(combo);
  mainlayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
  mainlayout->setContentsMargins(0, 0, 0, 0);
  QVariant vcombo;
  vcombo.setValue(static_cast<void*>(combo));
  funCheck->setProperty("FuncCombo", vcombo);
  QVariant veditor;
  veditor.setValue(static_cast<void*>(valeditor));
  funCheck->setProperty("FuncEditor", veditor);

  QObject::connect(funCheck, SIGNAL(toggled(bool)), this, SLOT(displayExpressionWidget(bool)));
  funCheck->setChecked(inputitem->isExpression(elementIdx));
  return checkFrame;
}

void qtInputsItem::displayExpressionWidget(bool checkstate)
{
  QToolButton* const funCheck = qobject_cast<QToolButton*>(QObject::sender());
  if (!funCheck)
  {
    return;
  }

  int elementIdx = funCheck->property("ElementIndex").toInt();
  auto inputitem = m_itemInfo.itemAs<ValueItem>();
  ResourcePtr lAttResource = inputitem->attribute()->attributeResource();
  if (!inputitem)
  {
    return;
  }
  QComboBox* combo = static_cast<QComboBox*>(funCheck->property("FuncCombo").value<void*>());
  QWidget* funcEditor = static_cast<QWidget*>(funCheck->property("FuncEditor").value<void*>());

  if (!combo || !funcEditor)
  {
    return;
  }

  if (checkstate)
  {
    combo->blockSignals(true);
    combo->clear();
    auto valItemDef = inputitem->definitionAs<ValueItemDefinition>();
    smtk::attribute::DefinitionPtr attDef = valItemDef->expressionDefinition();
    QStringList attNames;
    if (attDef)
    {
      std::vector<smtk::attribute::AttributePtr> result;
      lAttResource->findAttributes(attDef, result);
      std::vector<smtk::attribute::AttributePtr>::iterator it;
      for (it = result.begin(); it != result.end(); ++it)
      {
        attNames.push_back((*it)->name().c_str());
      }
      if (attNames.count() > 0)
      {
        attNames.sort();
        combo->addItems(attNames);
      }
    }

    int setIndex = -1;
    if (inputitem->isExpression(elementIdx))
    {
      smtk::attribute::RefItemPtr item = inputitem->expressionReference(elementIdx);
      if (item && (item->definition() != nullptr))
      {
        setIndex = attNames.indexOf(item->valueAsString(elementIdx).c_str());
      }
      else
      {
        // Not pointing to valid item - reset it
        this->unsetValue(elementIdx);
      }
    }
    else
    {
      // Did the user have a previously set expression?
      QVariant prevExpression;
      prevExpression = combo->property("PreviousValue");
      if (prevExpression.isValid())
      {
        QString expName = prevExpression.toString();
        AttributePtr attPtr = lAttResource->findAttribute(expName.toStdString());
        if (attPtr)
        {
          setIndex = attNames.indexOf(expName);
          inputitem->setExpression(elementIdx, attPtr);
          emit this->modified();
        }
        else
        {
          this->unsetValue(elementIdx);
        }
      }
      else if (inputitem->isSet(elementIdx))
      {
        this->unsetValue(elementIdx);
      }
    }
    combo->setCurrentIndex(setIndex);
    combo->blockSignals(false);
  }
  else
  {
    // OK - so now we need to deal with going from an expression to
    // to a constant - First , was the item an expression?  If so
    // we want to save the value in case they change their minds later
    if (inputitem->isExpression(elementIdx))
    {
      smtk::attribute::RefItemPtr item = inputitem->expressionReference(elementIdx);
      QVariant prevExpression(item->valueAsString(elementIdx).c_str());
      combo->setProperty("PreviousValue", prevExpression);
    }
    // Next - tell the edit box to update the item
    this->onInputValueChanged(funcEditor);
  }

  funcEditor->setVisible(!checkstate);
  combo->setVisible(checkstate);
}

void qtInputsItem::onExpressionReferenceChanged()
{
  QComboBox* const comboBox = qobject_cast<QComboBox*>(QObject::sender());
  if (!comboBox)
  {
    return;
  }
  int curIdx = comboBox->currentIndex();
  int elementIdx = comboBox->property("ElementIndex").toInt();
  auto inputitem = m_itemInfo.itemAs<ValueItem>();
  if (!inputitem)
  {
    return;
  }
  smtk::attribute::RefItemPtr item = inputitem->expressionReference(elementIdx);
  if (!item)
  {
    return;
  }

  if (curIdx >= 0)
  {
    ResourcePtr lAttResource = item->attribute()->attributeResource();
    AttributePtr attPtr = lAttResource->findAttribute(comboBox->currentText().toStdString());
    if (elementIdx >= 0 && inputitem->isSet(elementIdx) &&
      attPtr == inputitem->expression(elementIdx))
    {
      return; // nothing to do
    }

    if (attPtr)
    {
      inputitem->setExpression(elementIdx, attPtr);
    }
    else
    {
      item->unset(elementIdx);
      inputitem->unset(elementIdx);
    }
  }
  else
  {
    item->unset(elementIdx);
    inputitem->unset(elementIdx);
  }

  auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
  if (iview)
  {
    iview->valueChanged(inputitem->shared_from_this());
  }
  emit this->modified();
}

QWidget* qtInputsItem::createDoubleWidget(
  int elementIdx, ValueItemPtr vitem, QWidget* pWidget, QString& tooltip)
{
  auto dDef = vitem->definitionAs<DoubleItemDefinition>();
  auto ditem = dynamic_pointer_cast<DoubleItem>(vitem);
  double minVal, maxVal, defVal;

  // Let get the range of the item
  minVal = smtk_DOUBLE_MIN;
  if (dDef->hasMinRange())
  {
    minVal = dDef->minRange();
    if (!dDef->minRangeInclusive())
    {
      double multiplier = minVal >= 0 ? 1. : -1.;
      double to = multiplier * minVal * 1.001 + 1;
      minVal = nextafter(minVal, to);
    }
    QString inclusive = dDef->minRangeInclusive() ? "Inclusive" : "Not Inclusive";
    tooltip.append("Min(").append(inclusive).append("): ").append(
      QString::number(dDef->minRange()));
  }

  maxVal = smtk_DOUBLE_MAX;
  if (dDef->hasMaxRange())
  {
    maxVal = dDef->maxRange();
    if (!dDef->maxRangeInclusive())
    {
      double multiplier = maxVal >= 0 ? -1. : 1.;
      double to = multiplier * maxVal * 1.001 - 1;
      maxVal = nextafter(maxVal, to);
    }
    if (!tooltip.isEmpty())
    {
      tooltip.append("; ");
    }
    QString inclusive = dDef->maxRangeInclusive() ? "Inclusive" : "Not Inclusive";
    tooltip.append("Max(").append(inclusive).append("): ").append(
      QString::number(dDef->maxRange()));
  }

  // Lets get default info
  if (dDef->hasDefault())
  {
    defVal = dDef->defaultValue(elementIdx);
    if (!tooltip.isEmpty())
    {
      tooltip.append("; ");
    }
    tooltip.append("Default: ").append(QString::number(defVal));
  }

  // What type of option are we suppose to use
  std::string option = "LineEdit";
  // By default we should be using a line edit, lets see if
  // we were told what to use
  m_itemInfo.component().attribute("Option", option);

  if (option == "LineEdit")
  {
    auto editBox = new qtDoubleLineEdit(pWidget);
    editBox->setUseGlobalPrecisionAndNotation(false);
    std::string notation("Mixed");
    m_itemInfo.component().attribute("Notation", notation);
    if (notation == "Fixed")
    {
      editBox->setNotation(qtDoubleLineEdit::FixedNotation);
    }
    else if (notation == "Scientific")
    {
      editBox->setNotation(qtDoubleLineEdit::ScientificNotation);
    }

    qtDoubleValidator* validator = new qtDoubleValidator(this, elementIdx, editBox, pWidget);

    editBox->setValidator(validator);
    int widthValue = 100; // Default fixed width
    int precision = 0;
    m_itemInfo.component().attributeAsInt("FixedWidth", widthValue);
    if (widthValue > 0)
    {
      editBox->setFixedWidth(widthValue);
    }
    m_itemInfo.component().attributeAsInt("Precision", precision);
    if (precision > 0)
    {
      editBox->setPrecision(precision);
    }
    validator->setBottom(minVal);
    validator->setTop(maxVal);
    if (vitem->isSet(elementIdx))
    {
      if (this->Internals->m_editPrecision > 0)
      {
        editBox->setText(
          QString::number(ditem->value(elementIdx), 'f', this->Internals->m_editPrecision));
      }
      else
      {
        editBox->setText(vitem->valueAsString(elementIdx).c_str());
      }
    }
    return editBox;
  }

  if (option == "SpinBox")
  {
    QDoubleSpinBox* spinbox = new QDoubleSpinBox(pWidget);
    spinbox->setMaximum(maxVal);
    spinbox->setMinimum(minVal);
    double step;
    int decimals;
    if (!dDef->units().empty())
    {
      QString ustring = " ";
      ustring.append(dDef->units().c_str());
      spinbox->setSuffix(ustring);
    }
    if (m_itemInfo.component().attributeAsDouble("StepSize", step))
    {
      spinbox->setSingleStep(step);
    }
    if (m_itemInfo.component().attributeAsInt("Decimals", decimals))
    {
      spinbox->setDecimals(decimals);
    }
    if (ditem->isSet(elementIdx))
    {
      spinbox->setValue(ditem->value(elementIdx));
    }
    connect(spinbox, SIGNAL(valueChanged(double)), this, SLOT(doubleValueChanged(double)));
    return spinbox;
  }
  return nullptr;
}

QWidget* qtInputsItem::createIntWidget(
  int elementIdx, ValueItemPtr vitem, QWidget* pWidget, QString& tooltip)
{
  auto iDef = vitem->definitionAs<IntItemDefinition>();
  int minVal, maxVal, defVal;

  // Let get the range of the item
  minVal = smtk_INT_MIN;
  if (iDef->hasMinRange())
  {
    minVal = iDef->minRangeInclusive() ? iDef->minRange() : iDef->minRange() + 1;
    QString inclusive = iDef->minRangeInclusive() ? "Inclusive" : "Not Inclusive";
    tooltip.append("Min(").append(inclusive).append("): ").append(
      QString::number(iDef->minRange()));
  }

  maxVal = smtk_INT_MAX;
  if (iDef->hasMaxRange())
  {
    maxVal = iDef->maxRangeInclusive() ? iDef->maxRange() : iDef->maxRange() - 1;
    if (!tooltip.isEmpty())
    {
      tooltip.append("; ");
    }
    QString inclusive = iDef->maxRangeInclusive() ? "Inclusive" : "Not Inclusive";
    tooltip.append("Max(").append(inclusive).append("): ").append(
      QString::number(iDef->maxRange()));
  }

  // Lets get default info
  if (iDef->hasDefault())
  {
    defVal = iDef->defaultValue(elementIdx);
    if (!tooltip.isEmpty())
    {
      tooltip.append("; ");
    }
    tooltip.append("Default: ").append(QString::number(defVal));
  }

  // What type of option are we suppose to use
  std::string option = "LineEdit";
  // By default we should be using a line edit, lets see if
  // we were told what to use
  m_itemInfo.component().attribute("Option", option);

  if (option == "LineEdit")
  {
    QLineEdit* editBox = new QLineEdit(pWidget);
    qtIntValidator* validator = new qtIntValidator(this, elementIdx, editBox, pWidget);

    editBox->setValidator(validator);
    int widthValue = 100; // Default fixed width
    m_itemInfo.component().attributeAsInt("FixedWidth", widthValue);
    if (widthValue > 0)
    {
      editBox->setFixedWidth(widthValue);
    }
    validator->setBottom(minVal);
    validator->setTop(maxVal);
    if (vitem->isSet(elementIdx))
    {
      editBox->setText(vitem->valueAsString(elementIdx).c_str());
    }
    return editBox;
  }

  if (option == "SpinBox")
  {
    QSpinBox* spinbox = new QSpinBox(pWidget);
    auto iitem = dynamic_pointer_cast<IntItem>(vitem);
    spinbox->setMaximum(maxVal);
    spinbox->setMinimum(minVal);
    int step;
    if (!iDef->units().empty())
    {
      QString ustring = " ";
      ustring.append(iDef->units().c_str());
      spinbox->setSuffix(ustring);
    }
    if (m_itemInfo.component().attributeAsInt("StepSize", step))
    {
      spinbox->setSingleStep(step);
    }
    if (iitem->isSet(elementIdx))
    {
      spinbox->setValue(iitem->value(elementIdx));
    }
    connect(spinbox, SIGNAL(valueChanged(int)), this, SLOT(intValueChanged(int)));
    return spinbox;
  }
  return nullptr;
}

QWidget* qtInputsItem::createEditBox(int elementIdx, QWidget* pWidget)
{
  auto item = m_itemInfo.itemAs<ValueItem>();
  qtUIManager* uimanager = m_itemInfo.uiManager();
  if (!item)
  {
    return NULL;
  }

  QWidget* inputWidget = NULL;
  QVariant vdata(elementIdx);
  QString tooltip;

  // If the item is not set but has a default lets use it
  if (!item->isSet(elementIdx) && item->hasDefault())
  {
    item->setToDefault(elementIdx);
    emit this->modified();
  }

  switch (item->type())
  {
    case smtk::attribute::Item::DoubleType:
    {
      inputWidget = this->createDoubleWidget(elementIdx, item, pWidget, tooltip);
      break;
    }
    case smtk::attribute::Item::IntType:
    {
      inputWidget = this->createIntWidget(elementIdx, item, pWidget, tooltip);
      break;
    }
    case smtk::attribute::Item::StringType:
    {
      auto sDef = item->definitionAs<StringItemDefinition>();
      auto sitem = dynamic_pointer_cast<StringItem>(item);
      QString valText;
      if (item->isSet(elementIdx))
      {
        valText = item->valueAsString(elementIdx).c_str();
      }

      if (sDef->isMultiline())
      {
        qtTextEdit* textEdit = new qtTextEdit(pWidget);
        textEdit->setPlainText(valText);
        QObject::connect(textEdit, SIGNAL(textChanged()), this, SLOT(onTextEditChanged()));
        inputWidget = textEdit;
      }
      else
      {
        QLineEdit* lineEdit = new QLineEdit(pWidget);
        int widthValue = 0; // Default no fixed width
        m_itemInfo.component().attributeAsInt("FixedWidth", widthValue);
        if (widthValue > 0)
        {
          lineEdit->setFixedWidth(widthValue);
        }
        if (sitem->isSecure())
        {
          lineEdit->setEchoMode(QLineEdit::Password);
        }

        lineEdit->setText(valText);
        inputWidget = lineEdit;
      }
      //      inputWidget->setMinimumWidth(100);
      inputWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      if (sDef->hasDefault())
      {
        tooltip.append("Default: ").append(sDef->defaultValue(elementIdx).c_str());
      }
      break;
    }
    default:
      //m_errorStatus << "Error: Unsupported Item Type: " <<
      // smtk::attribute::Item::type2String(item->type()) << "\n";
      break;
  }

  if (!inputWidget)
  {
    return NULL;
  }

  inputWidget->setProperty("ElementIndex", vdata);

  // Lets determine the item's state
  if (!item->isSet(elementIdx))
  {
    uimanager->setWidgetColorToInvalid(inputWidget);
  }
  else if (item->isUsingDefault(elementIdx))
  {
    uimanager->setWidgetColorToDefault(inputWidget);
  }
  else
  {
    uimanager->setWidgetColorToNormal(inputWidget);
  }

  if (!tooltip.isEmpty())
  {
    inputWidget->setToolTip(tooltip);
  }
  if (QLineEdit* const editBox = qobject_cast<QLineEdit*>(inputWidget))
  {
    QObject::connect(editBox, SIGNAL(textChanged(const QString&)), this, SLOT(onLineEditChanged()),
      Qt::QueuedConnection);
    QObject::connect(
      editBox, SIGNAL(editingFinished()), this, SLOT(onLineEditFinished()), Qt::QueuedConnection);
  }

  return inputWidget;
}

void qtInputsItem::onTextEditChanged()
{
  this->onInputValueChanged(QObject::sender());
}

void qtInputsItem::onLineEditChanged()
{
  // Here we only handle changes when this is invoked from setText()
  // which is normally used programatically, and the setText() will have
  // modified flag reset to false;
  QLineEdit* const editBox = qobject_cast<QLineEdit*>(QObject::sender());
  if (!editBox)
  {
    return;
  }
  // If this is not from setText(), ignore it. We are using editingFinished
  // signal to handle others.
  if (editBox->isModified())
  {
    return;
  }

  this->onInputValueChanged(editBox);
}

void qtInputsItem::onLineEditFinished()
{
  this->onInputValueChanged(QObject::sender());
}

void qtInputsItem::doubleValueChanged(double newVal)
{
  auto obj = QObject::sender();
  auto senderWidget = qobject_cast<QWidget*>(obj);
  auto ditem = m_itemInfo.itemAs<DoubleItem>();
  if (!ditem)
  {
    return;
  }

  int elementIdx = obj->property("ElementIndex").toInt();
  bool isDefault = false;
  bool valChanged = false;
  bool isInvalid = false;

  double val = ditem->value(elementIdx);
  if ((ditem->isExpression(elementIdx) || !ditem->isSet(elementIdx)) || val != newVal)
  {
    ditem->setValue(elementIdx, newVal);
    valChanged = true;
  }
  // Lets determine if the item is set to the default value -
  isDefault = ditem->isUsingDefault(elementIdx);
  isInvalid = !ditem->isSet(elementIdx);
  auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
  if (valChanged)
  {
    if (iview)
    {
      iview->valueChanged(ditem);
    }
    emit this->modified();
  }
  if (iview)
  {
    qtUIManager* uimanager = iview->uiManager();
    isDefault ? uimanager->setWidgetColorToDefault(senderWidget)
              : (isInvalid ? uimanager->setWidgetColorToInvalid(senderWidget)
                           : uimanager->setWidgetColorToNormal(senderWidget));
  }
}

void qtInputsItem::intValueChanged(int newVal)
{
  auto obj = QObject::sender();
  auto senderWidget = qobject_cast<QWidget*>(obj);
  auto iitem = m_itemInfo.itemAs<IntItem>();
  if (!iitem)
  {
    return;
  }

  int elementIdx = obj->property("ElementIndex").toInt();
  bool isDefault = false;
  bool valChanged = false;
  bool isInvalid = false;

  int val = iitem->value(elementIdx);
  if ((iitem->isExpression(elementIdx) || !iitem->isSet(elementIdx)) || val != newVal)
  {
    iitem->setValue(elementIdx, newVal);
    valChanged = true;
  }
  // Lets determine if the item is set to the default value -
  isDefault = iitem->isUsingDefault(elementIdx);
  isInvalid = !iitem->isSet(elementIdx);
  auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
  if (valChanged)
  {
    if (iview)
    {
      iview->valueChanged(iitem);
    }
    emit this->modified();
  }
  if (iview)
  {
    qtUIManager* uimanager = iview->uiManager();
    isDefault ? uimanager->setWidgetColorToDefault(senderWidget)
              : (isInvalid ? uimanager->setWidgetColorToInvalid(senderWidget)
                           : uimanager->setWidgetColorToNormal(senderWidget));
  }
}

void qtInputsItem::onInputValueChanged(QObject* obj)
{
  QLineEdit* const editBox = qobject_cast<QLineEdit*>(obj);
  QTextEdit* const textBox = qobject_cast<QTextEdit*>(obj);
  if (!editBox && !textBox)
  {
    return;
  }
  QWidget* inputBox;
  if (editBox != NULL)
  {
    inputBox = editBox;
  }
  else
  {
    inputBox = textBox;
  }
  auto rawitem = m_itemInfo.itemAs<ValueItem>();
  if (!rawitem)
  {
    return;
  }

  int elementIdx =
    editBox ? editBox->property("ElementIndex").toInt() : textBox->property("ElementIndex").toInt();
  bool isDefault = false;
  bool valChanged = false;
  bool isInvalid = false;
  if (editBox && !editBox->text().isEmpty())
  {
    if (rawitem->type() == smtk::attribute::Item::DoubleType)
    {
      auto ditem = dynamic_pointer_cast<DoubleItem>(rawitem);
      double val = ditem->value(elementIdx);
      if ((rawitem->isExpression(elementIdx) || !rawitem->isSet(elementIdx)) ||
        val != editBox->text().toDouble())
      {
        ditem->setValue(elementIdx, editBox->text().toDouble());
        valChanged = true;
      }
    }
    else if (rawitem->type() == smtk::attribute::Item::IntType)
    {
      auto iitem = dynamic_pointer_cast<IntItem>(rawitem);
      int val = iitem->value(elementIdx);
      if ((rawitem->isExpression(elementIdx) || !rawitem->isSet(elementIdx)) ||
        val != editBox->text().toInt())
      {
        iitem->setValue(elementIdx, editBox->text().toInt());
        valChanged = true;
      }
    }
    else if (rawitem->type() == smtk::attribute::Item::StringType)
    {
      auto sitem = dynamic_pointer_cast<StringItem>(rawitem);
      std::string val = sitem->value(elementIdx);
      if ((rawitem->isExpression(elementIdx) || !rawitem->isSet(elementIdx)) ||
        val != editBox->text().toStdString())
      {
        sitem->setValue(elementIdx, editBox->text().toStdString());
        valChanged = true;
      }
    }
    else
    {
      rawitem->unset(elementIdx);
      valChanged = true;
    }
  }
  else if (textBox && !textBox->toPlainText().isEmpty() &&
    rawitem->type() == smtk::attribute::Item::StringType)
  {
    auto sitem = dynamic_pointer_cast<StringItem>(rawitem);
    std::string val = sitem->value(elementIdx);
    if ((rawitem->isExpression(elementIdx) || !rawitem->isSet(elementIdx)) ||
      val != textBox->toPlainText().toStdString())
    {
      sitem->setValue(elementIdx, textBox->toPlainText().toStdString());
      valChanged = true;
    }
  }
  else
  {
    // OK so the widget is empty - do we have to unset the item?
    if (rawitem->isSet(elementIdx))
    {
      rawitem->unset(elementIdx);
      valChanged = true;
    }
    isInvalid = true;
  }
  // Lets determine if the item is set to the default value -
  isDefault = rawitem->isUsingDefault(elementIdx);
  isInvalid = !rawitem->isSet(elementIdx);
  auto iview = dynamic_cast<qtBaseAttributeView*>(m_itemInfo.baseView().data());
  if (valChanged)
  {
    if (iview)
    {
      iview->valueChanged(rawitem->shared_from_this());
    }
    emit this->modified();
  }
  if (iview)
  {
    qtUIManager* uimanager = iview->uiManager();
    isDefault ? uimanager->setWidgetColorToDefault(inputBox)
              : (isInvalid ? uimanager->setWidgetColorToInvalid(inputBox)
                           : uimanager->setWidgetColorToNormal(inputBox));
  }
}

void qtInputsItem::onChildItemModified()
{
  emit this->modified();
}
