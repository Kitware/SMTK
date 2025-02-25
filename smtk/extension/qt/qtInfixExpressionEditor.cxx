//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtInfixExpressionEditor.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/io/Logger.h"

#include <string>

#include <QFrame>
#include <QGridLayout>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QToolButton>
#include <QVBoxLayout>

using namespace smtk::attribute;

namespace smtk
{
namespace extension
{

class qtInfixExpressionEditorInternals
{
public:
  qtInfixExpressionEditorInternals() = default;

  QLabel* mp_nameLabel{ nullptr };
  QGridLayout* mp_entryLayout{ nullptr };

  QToolButton* mp_addItemButton{ nullptr };

  // Provides lookup to minus button's associated UI elements when an item is
  // removed.
  QHash<QToolButton*, QPair<QHBoxLayout*, qtInfixExpressionEditorRow*>> m_removeButtonToRow;
  QList<QToolButton*> m_minusButtonIndices;
};

smtk::extension::qtItem* smtk::extension::qtInfixExpressionEditor::createItemWidget(
  const smtk::extension::qtAttributeItemInfo& info)
{
  if (!info.itemAs<smtk::attribute::StringItem>())
  {
    return nullptr;
  }

  return new qtInfixExpressionEditor(info);
}

qtInfixExpressionEditor::qtInfixExpressionEditor(const qtAttributeItemInfo& info)
  : qtItem(info)
{
  mp_internals = new qtInfixExpressionEditorInternals;
  this->createWidget();
}

qtInfixExpressionEditor::~qtInfixExpressionEditor()
{
  delete mp_internals;
}

void qtInfixExpressionEditor::setLabelVisible(bool visible)
{
  mp_internals->mp_nameLabel->setVisible(visible);
}

// TODO: when would this be fixed width?
bool qtInfixExpressionEditor::isFixedWidth() const
{
  return false;
}

void qtInfixExpressionEditor::updateItemData()
{
  StringItemPtr theItem = itemAs<StringItem>();
  if (!theItem)
  {
    return;
  }

  for (auto& pair : mp_internals->m_removeButtonToRow)
  {
    qtInfixExpressionEditorRow* editor = pair.second;
    if (!editor)
      continue;

    int elementIdx = editor->itemElementIndex();

    bool isValid = theItem->isSet(static_cast<size_t>(elementIdx));
    bool isDefault = theItem->isUsingDefault(static_cast<size_t>(elementIdx));
    QLineEdit* inputBox = editor->editBox();

    QSignalBlocker blocker(inputBox);

    // We manually call the slot to update the result line edit of |editor|
    // because signals are blocked.

    if (isValid && theItem->valueAsString(elementIdx).c_str() != inputBox->text())
    {
      inputBox->setText(theItem->valueAsString(elementIdx).c_str());
      editor->onEditBoxChanged(theItem->valueAsString(elementIdx).c_str());
    }
    else if (!isValid && inputBox->text() != QString())
    {
      inputBox->setText(QString());
      editor->onEditBoxChanged(QString());
    }

    qtUIManager* uimanager = uiManager();
    isDefault ? uimanager->setWidgetColorToDefault(inputBox)
              : (isValid ? uimanager->setWidgetColorToNormal(inputBox)
                         : uimanager->setWidgetColorToInvalid(inputBox));
  }

  this->qtItem::updateItemData();
}

void qtInfixExpressionEditor::onInputValueChanged(const QString& text, int elementIdx)
{
  StringItemPtr theItem = itemAs<StringItem>();
  if (!theItem)
  {
    return;
  }

  const std::string currentValue = theItem->value(elementIdx);
  bool valChanged = false;
  if (!theItem->isSet(elementIdx) || currentValue.c_str() != text)
  {
    theItem->setValue(elementIdx, text.toStdString());
    valChanged = true;
  }

  bool isInvalid = !theItem->isSet(elementIdx);
  bool isDefault = theItem->isUsingDefault(elementIdx);
  qtBaseAttributeView* baseView = m_itemInfo.baseView();
  if (valChanged)
  {
    if (baseView)
    {
      baseView->valueChanged(theItem);
    }
    Q_EMIT this->modified(this);
  }

  // Set the widget color of the editBox on this row using |baseView|'s
  // qtUIManager, if available.
  if (baseView)
  {
    qtInfixExpressionEditorRow* row = qobject_cast<qtInfixExpressionEditorRow*>(QObject::sender());
    if (row)
    {
      QLineEdit* inputBox = row->editBox();
      if (inputBox)
      {
        qtUIManager* uimanager = baseView->uiManager();

        isDefault ? uimanager->setWidgetColorToDefault(inputBox)
                  : (isInvalid ? uimanager->setWidgetColorToInvalid(inputBox)
                               : uimanager->setWidgetColorToNormal(inputBox));
      }
    }
  }
}

void qtInfixExpressionEditor::createWidget()
{
  this->clearChildWidgets();
  this->updateUI();
}

void qtInfixExpressionEditor::loadInputValues()
{
  StringItemPtr theItem = itemAs<StringItem>();
  if (!theItem)
  {
    return;
  }

  int numOfValues = static_cast<int>(theItem->numberOfValues());
  if (numOfValues == 0 && !theItem->isExtensible())
  {
    return;
  }

  if (theItem->isExtensible())
  {
    if (!mp_internals->mp_addItemButton)
    {
      QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      mp_internals->mp_addItemButton = new QToolButton(this->widget());
      QString iconName(":/icons/attribute/plus.png");
      mp_internals->mp_addItemButton->setText("Add New Value");
      mp_internals->mp_addItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
      mp_internals->mp_addItemButton->setIcon(QIcon(iconName));
      mp_internals->mp_addItemButton->setSizePolicy(sizeFixedPolicy);
      connect(
        mp_internals->mp_addItemButton,
        &QToolButton::clicked,
        this,
        &qtInfixExpressionEditor::onAddNewValue);
      mp_internals->mp_entryLayout->addWidget(mp_internals->mp_addItemButton, 0, 1);
    }
  }

  for (int i = 0; i < numOfValues; ++i)
  {
    this->addInputEditor(i);
  }
}

void qtInfixExpressionEditor::clearChildWidgets()
{
  StringItemPtr theItem = itemAs<StringItem>();
  if (!theItem)
  {
    return;
  }

  if (theItem->isExtensible())
  {
    for (QToolButton* tButton : mp_internals->m_removeButtonToRow.keys())
    {
      delete mp_internals->m_removeButtonToRow.value(tButton).first;
      delete tButton;
    }
    mp_internals->m_removeButtonToRow.clear();
    mp_internals->m_minusButtonIndices.clear();
  }
}

void qtInfixExpressionEditor::addInputEditor(int i)
{
  StringItemPtr theItem = itemAs<StringItem>();
  if (!theItem)
  {
    return;
  }

  int numOfValues = static_cast<int>(theItem->numberOfValues());
  if (numOfValues == 0)
  {
    return;
  }

  qtInfixExpressionEditorRow* rowEditor = createInputWidget(i);
  if (!rowEditor)
  {
    return;
  }

  // This connection must NOT be a QueuedConnection so that |rowEditor|'s
  // evaluator can use the updated StringItem's value as processed by
  // onInputValueChanged.
  connect(
    rowEditor,
    &qtInfixExpressionEditorRow::editBoxChanged,
    this,
    &qtInfixExpressionEditor::onInputValueChanged,
    Qt::DirectConnection);

  ConstStringItemDefinitionPtr itemDef = theItem->definitionAs<StringItemDefinition>();
  QHBoxLayout* editorLayout = new QHBoxLayout(this->widget());
  editorLayout->setMargin(0);
  editorLayout->setSpacing(3);
  if (theItem->isExtensible())
  {
    QToolButton* minusButton = new QToolButton(this->widget());
    QString iconName(":/icons/attribute/minus.png");
    minusButton->setFixedSize(QSize(12, 12));
    minusButton->setIcon(QIcon(iconName));
    minusButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    minusButton->setToolTip("Remove value");
    editorLayout->addWidget(minusButton);
    connect(minusButton, &QToolButton::clicked, this, &qtInfixExpressionEditor::onRemoveValue);
    QPair<QHBoxLayout*, qtInfixExpressionEditorRow*> p(editorLayout, rowEditor);
    mp_internals->m_removeButtonToRow[minusButton] = p;
    mp_internals->m_minusButtonIndices.push_back(minusButton);
  }

  if (numOfValues > 1 && itemDef->hasValueLabels())
  {
    std::string componentLabel = itemDef->valueLabel(i);
    if (!componentLabel.empty())
    {
      QString labelText = componentLabel.c_str();
      QLabel* label = new QLabel(labelText, rowEditor);
      label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      editorLayout->addWidget(label);
    }
  }

  editorLayout->addWidget(rowEditor);

  // Skip the "Add New Value" button in the first row.
  int row = theItem->isExtensible() ? i + 1 : i;
  mp_internals->mp_entryLayout->addLayout(editorLayout, row, 1);

  // Note qtInputsItem has an orientation member which determines the orientation
  // when an item has component labels.
  this->updateExtensibleState();
}

// IGNORING:
// BriefDescription
// Units
// Optional
void qtInfixExpressionEditor::updateUI()
{
  StringItemPtr theItem = itemAs<StringItem>();
  if (!theItem)
  {
    return;
  }

  qtBaseAttributeView* baseView = m_itemInfo.baseView();
  if (!baseView || !baseView->displayItem(theItem))
  {
    return;
  }

  ConstStringItemDefinitionPtr itemDef = theItem->definitionAs<StringItemDefinition>();

  m_widget = new QFrame(parentWidget());
  m_widget->setObjectName(theItem->name().c_str());

  mp_internals->mp_entryLayout = new QGridLayout(m_widget);
  mp_internals->mp_entryLayout->setMargin(0);
  mp_internals->mp_entryLayout->setSpacing(0);
  mp_internals->mp_entryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QString labelText;
  if (!theItem->label().empty())
  {
    labelText = theItem->label().c_str();
  }
  else
  {
    labelText = theItem->name().c_str();
  }
  mp_internals->mp_nameLabel = new QLabel(labelText, m_widget);
  mp_internals->mp_nameLabel->setSizePolicy(sizeFixedPolicy);
  if (baseView)
  {
    int requiredLen = m_itemInfo.uiManager()->getWidthOfText(
      theItem->label(), m_itemInfo.uiManager()->advancedFont());
    int labLen = baseView->fixedLabelWidth();
    if ((requiredLen / 2) > labLen)
    {
      labLen = requiredLen;
    }
    mp_internals->mp_nameLabel->setFixedWidth(labLen);
  }
  mp_internals->mp_nameLabel->setWordWrap(true);
  mp_internals->mp_nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  loadInputValues();

  mp_internals->mp_entryLayout->addWidget(mp_internals->mp_nameLabel, 0, 0);
  if (parentWidget() && parentWidget()->layout())
  {
    parentWidget()->layout()->addWidget(m_widget);
  }
}

void qtInfixExpressionEditor::updateExtensibleState()
{
  StringItemPtr theItem = itemAs<StringItem>();
  if (!theItem || !theItem->isExtensible())
  {
    return;
  }
  bool maxReached = (theItem->maxNumberOfValues() > 0) &&
    (theItem->maxNumberOfValues() == theItem->numberOfValues());
  mp_internals->mp_addItemButton->setEnabled(!maxReached);

  bool minReached = (theItem->numberOfRequiredValues() > 0) &&
    (theItem->numberOfRequiredValues() == theItem->numberOfValues());
  for (QToolButton* button : mp_internals->m_removeButtonToRow.keys())
  {
    button->setEnabled(!minReached);
  }
}

qtInfixExpressionEditorRow* qtInfixExpressionEditor::createInputWidget(int elementIdx)
{
  StringItemPtr theItem = itemAs<StringItem>();
  if (!theItem)
  {
    return nullptr;
  }

  QString valText;
  if (theItem->isSet(static_cast<std::size_t>(elementIdx)))
  {
    valText = theItem->valueAsString(static_cast<std::size_t>(elementIdx)).c_str();
  }

  smtk::attribute::AttributePtr att = theItem->attribute();
  std::unique_ptr<Evaluator> evaluator = att->createEvaluator();
  qtInfixExpressionEditorRow* row =
    new qtInfixExpressionEditorRow(valText, elementIdx, std::move(evaluator), widget());

  if (!theItem->isSet(static_cast<std::size_t>(elementIdx)))
  {
    uiManager()->setWidgetColorToInvalid(row->editBox());
  }
  else if (theItem->isUsingDefault(static_cast<std::size_t>(elementIdx)))
  {
    uiManager()->setWidgetColorToDefault(row->editBox());
  }
  else
  {
    uiManager()->setWidgetColorToNormal(row->editBox());
  }

  return row;
}

void qtInfixExpressionEditor::onAddNewValue()
{
  StringItemPtr theItem = itemAs<StringItem>();
  if (!theItem)
  {
    return;
  }

  if (theItem->setNumberOfValues(theItem->numberOfValues() + 1))
  {
    this->addInputEditor(static_cast<int>(theItem->numberOfValues()) - 1);
  }
  Q_EMIT this->modified(this);
}

void qtInfixExpressionEditor::onRemoveValue()
{
  QToolButton* const minusButton = qobject_cast<QToolButton*>(QObject::sender());
  if (!minusButton || !mp_internals->m_removeButtonToRow.contains(minusButton))
  {
    return;
  }

  int gIdx = mp_internals->m_minusButtonIndices.indexOf(minusButton);
  StringItemPtr theItem = itemAs<StringItem>();
  if (!theItem || gIdx < 0 || gIdx >= static_cast<int>(theItem->numberOfValues()))
  {
    return;
  }

  if (!theItem->removeValue(gIdx))
  {
    return;
  }

  this->clearChildWidgets();
  this->loadInputValues();
  Q_EMIT this->modified(this);
}

} // namespace extension
} // namespace smtk
