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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
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
#include "smtk/attribute/operators/Signal.h"
#include "smtk/attribute/utility/Queries.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/StringUtil.h"

#include "smtk/extension/qt/qtAttributeEditorDialog.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include "smtk/extension/qt/qtDiscreteValueEditor.h"
#include "smtk/extension/qt/qtDoubleLineEdit.h"
#include "smtk/extension/qt/qtDoubleUnitsLineEdit.h"
#include "smtk/extension/qt/qtOverlay.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/io/Logger.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QDoubleValidator>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QSizePolicy>
#include <QSpinBox>
#include <QTextEdit>
#include <QTextStream>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>

#include "units/System.h"
#include "units/Unit.h"

#include <cmath>

#if defined(SMTK_MSVC) && _MSC_VER <= 1500
#include <float.h>
double nextafter(double x, double y)
{
  return _nextafter(x, y);
}
#endif

using namespace smtk::attribute;
using namespace smtk::extension;

namespace
{

// Returns the value in position |element| from ValueItem |item|, with errors
// reporting in |log|. Used by qtInputsItem's expression widgets. The returned
// QVariant will return true for QVariant::isNull if no value can be obtained.
// If |floatingPointPrecision| is greater than 0, that number of decimal places
// will be used for values from DoubleItems.
QVariant valueFromValueItemAsQVariant(
  const smtk::attribute::ValueItemPtr& item,
  int element,
  smtk::io::Logger& log,
  int floatingPointPrecision)
{
  QVariant result;

  if (item->type() == smtk::attribute::Item::IntType)
  {
    smtk::attribute::IntItemPtr intItem = std::dynamic_pointer_cast<smtk::attribute::IntItem>(item);
    result = intItem->value(element, log);
  }
  else if (item->type() == smtk::attribute::Item::DoubleType)
  {
    smtk::attribute::DoubleItemPtr doubleItem =
      std::dynamic_pointer_cast<smtk::attribute::DoubleItem>(item);
    // Limits the precision of a returned double.
    if (floatingPointPrecision > 0)
    {
      result = QString::number(doubleItem->value(element, log), 'g', 6);
    }
    else
    {
      result = std::to_string(doubleItem->value(element, log)).c_str();
    }
  }
  else if (item->type() == smtk::attribute::Item::StringType)
  {
    smtk::attribute::StringItemPtr stringItem =
      std::dynamic_pointer_cast<smtk::attribute::StringItem>(item);
    result = stringItem->value(element, log).c_str();
  }

  return result;
}

} // anonymous namespace

qtDoubleValidator::qtDoubleValidator(
  qtInputsItem* item,
  int elementIndex,
  QLineEdit* lineEdit,
  QObject* inParent)
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
    int defaultIdx =
      static_cast<int>(dDef->defaultValues().size()) <= m_elementIndex ? 0 : m_elementIndex;
    input = QString::number(dDef->defaultValue(defaultIdx));
  }
  else
  {
    m_item->uiManager()->setWidgetColorToInvalid(m_lineWidget);
  }
}

qtIntValidator::qtIntValidator(
  qtInputsItem* item,
  int elementIndex,
  QLineEdit* lineEdit,
  QObject* inParent)
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
    int defaultIdx =
      static_cast<int>(dDef->defaultValues().size()) <= m_elementIndex ? 0 : m_elementIndex;
    input = QString::number(dDef->defaultValue(defaultIdx));
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

  // VectorItemOrient stores the layout orientation (horizontal or vertical) for either
  // item values or children items. Defaults are:
  // 1.  Ordinary value items => values layout is horizontal
  // 2.  Extensible values => values layout is vertical
  // 3a. Discrete items with active children => children item layout is vertical EXCEPT:
  // 3b. Discrete item with each value assigned 0 or 1 child item with empty label =>
  //      children item layout is horizontal
  // Cases 1, 3a, 3b can by overridden with an ItemView attribute Layout="Horizontal"
  // or Layout="Vertical"
  Qt::Orientation VectorItemOrient;

  // for discrete items that with potential child widget
  // <Enum-Combo, child-layout >
  QMap<QWidget*, QPointer<QLayout>> ChildrenMap;

  // for extensible items
  QMap<QToolButton*, QPair<QPointer<QLayout>, QPointer<QWidget>>> ExtensibleMap;
  QList<QToolButton*> MinusButtonIndices;
  QPointer<QToolButton> AddItemButton;
  QList<QPointer<qtDiscreteValueEditor>> DiscreteEditors;
  QPointer<QCheckBox> OptionalCheck;
  QPointer<QFrame> m_valuesFrame;
  QPointer<QFrame> m_dataFrame;
  QPointer<QFrame> m_expressionFrame;
  QPointer<QToolButton> m_expressionButton;
  QPointer<QComboBox> m_expressionCombo;
  QPointer<QLabel> m_expressionEqualsLabel;
  QPointer<QLineEdit> m_expressionResultLineEdit;
  QString m_lastExpression;
  int m_editPrecision;

  // Store expression on/off state in lieu of widget visibility which isn't immediate
  bool m_usingExpression = false;
  QList<QWidget*> m_editors;
};

QWidget* qtInputsItem::lastEditor() const
{
  if (m_internals->m_usingExpression)
  {
    return m_internals->m_expressionCombo;
  }
  else if (m_internals->m_editors.isEmpty())
  {
    return nullptr;
  }
  return m_internals->m_editors.last();
}

void qtInputsItem::updateTabOrder(QWidget* precedingEditor)
{
  QWidget* previousEd = precedingEditor;

  // Expression editor
  if (m_internals->m_expressionCombo && m_internals->m_expressionCombo->isVisible())
  {
    QWidget::setTabOrder(previousEd, m_internals->m_expressionCombo);
    return;
  }

  // Value editor(s)
  for (QWidget* ed : m_internals->m_editors)
  {
    if (ed != nullptr) // needed?
    {
      QWidget::setTabOrder(previousEd, ed);
    }
    previousEd = ed;
  }
}

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
  m_internals = new qtInputsItemInternals;
  m_isLeafItem = true;
  m_internals->VectorItemOrient = this->getOrientation(info);
  m_internals->m_editPrecision = 0; // Use full precision by default
  // See if we are suppose to override it
  m_itemInfo.component().attributeAsInt("EditPrecision", m_internals->m_editPrecision);

  this->createWidget();
}

qtInputsItem::~qtInputsItem()
{
  delete m_internals;
}

void qtInputsItem::unsetValue(int elementIndex)
{
  auto item = m_itemInfo.itemAs<ValueItem>();
  if (item->isSet(elementIndex))
  {
    item->unset(elementIndex);
    Q_EMIT modified();
    auto* iview = m_itemInfo.baseView();
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
    Q_EMIT this->modified();
    auto* iview = m_itemInfo.baseView();
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
  Q_EMIT this->modified();
  auto* iview = m_itemInfo.baseView();
  if (iview)
  {
    iview->valueChanged(item);
  }
}

void qtInputsItem::setLabelVisible(bool visible)
{
  m_internals->theLabel->setVisible(visible);
}

void qtInputsItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = m_itemInfo.item();
  auto* iview = m_itemInfo.baseView();
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

  if (valItem->isOptional())
  {
    m_internals->OptionalCheck->setVisible(true);
    this->setOutputOptional(valItem->localEnabledState() ? 1 : 0);
  }
  else if (m_internals->OptionalCheck)
  {
    m_internals->OptionalCheck->setVisible(false);
    this->setOutputOptional(1);
  }
  if (valItem->isDiscrete())
  {
    // Ok we are dealing with discrete editors
    Q_FOREACH (QWidget* cwidget, m_internals->ChildrenMap.keys())
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
    Q_FOREACH (QWidget* cwidget, m_internals->ChildrenMap.keys())
    {
      this->updateDoubleItemData(cwidget, doubleItem);
    }
    this->qtItem::updateItemData();
    return;
  }
  auto intItem = m_itemInfo.itemAs<IntItem>();
  if (intItem != nullptr)
  {
    Q_FOREACH (QWidget* cwidget, m_internals->ChildrenMap.keys())
    {
      this->updateIntItemData(cwidget, intItem);
    }
    this->qtItem::updateItemData();
    return;
  }

  auto stringItem = m_itemInfo.itemAs<StringItem>();
  if (stringItem != nullptr)
  {
    Q_FOREACH (QWidget* cwidget, m_internals->ChildrenMap.keys())
    {
      this->updateStringItemData(cwidget, stringItem);
    }
    this->qtItem::updateItemData();
    return;
  }
  this->qtItem::updateItemData();
}

void qtInputsItem::updateDoubleItemData(
  QWidget* iwidget,
  const smtk::attribute::DoubleItemPtr& ditem)
{
  int elementIdx = iwidget->property("ElementIndex").toInt();
  QLineEdit* lineEdit = qobject_cast<QLineEdit*>(iwidget);
  auto* valueUnitsEditor = qobject_cast<qtDoubleUnitsLineEdit*>(iwidget);
  bool isValid = ditem->isSet(elementIdx);
  bool isDefault = ditem->isUsingDefault(elementIdx);

  if (valueUnitsEditor != nullptr)
  {
    QSignalBlocker blocker(valueUnitsEditor);
    if (isValid)
    {
      valueUnitsEditor->setText(ditem->valueAsString(elementIdx).c_str());
    }
    else
    {
      valueUnitsEditor->setText("");
    }
  }
  else if (lineEdit != nullptr)
  {
    QSignalBlocker blocker(lineEdit);
    QString ival;
    if (isValid)
    {
      if (m_internals->m_editPrecision > 0)
      {
        ival = QString::number(ditem->value(elementIdx), 'f', m_internals->m_editPrecision);
      }
      else
      {
        // We need to split the value string into 2 parts and only display the value
        // not the units
        std::string valStr, unitsStr;
        DoubleItemDefinition::splitStringStartingDouble(
          ditem->valueAsString(elementIdx), valStr, unitsStr);
        ival = valStr.c_str();
      }
      if (lineEdit->text() != ival)
      {
        lineEdit->setText(ival);
      }
    }
    else if (lineEdit->text() != "")
    {
      lineEdit->setText("");
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
  QWidget* iwidget,
  const smtk::attribute::StringItemPtr& sitem)
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

void qtInputsItem::updateExpressionRefWidgetForEvaluation(
  ValueItemPtr inputItem,
  bool showMessageBox)
{
  QString warningText;
  if (inputItem->isExpression())
  {
    std::unique_ptr<smtk::attribute::Evaluator> evaluator =
      inputItem->expression()->createEvaluator();
    if (evaluator)
    {
      size_t evaluatableElements = evaluator->numberOfEvaluatableElements();
      size_t inputItemValues = inputItem->numberOfValues();
      if (evaluatableElements > inputItemValues)
        warningText = "Warning: " + QString::number(evaluatableElements) +
          " elements given for evaluation, but this item only requires " +
          QString::number(inputItemValues) + ".";
    }
  }

  std::vector<smtk::io::Logger> logs;
  QStringList valueStrings;
  for (int i = 0; i < static_cast<int>(inputItem->numberOfValues()); ++i)
  {
    smtk::io::Logger currentLog;
    QVariant val =
      valueFromValueItemAsQVariant(inputItem, i, currentLog, m_internals->m_editPrecision);
    if (val.isNull())
    {
      // This is a diagnostic Record. The user ideally shouldn't see this.
      currentLog.addRecord(smtk::io::Logger::Error, "Couldn't get value.");
    }

    logs.push_back(currentLog);
    valueStrings << val.toString();
  }

  // Collects logs with errors along with their item indices.
  std::vector<std::pair<int, smtk::io::Logger>> logsWithErrors;
  for (int i = 0; i < static_cast<int>(logs.size()); ++i)
  {
    if (logs[i].hasErrors())
      logsWithErrors.emplace_back(std::make_pair(i, std::move(logs[i])));
  }

  if (!logsWithErrors.empty())
  {
    QString toolTipText;
    for (int i = 0; i < static_cast<int>(logsWithErrors.size()); ++i)
    {
      const smtk::io::Logger& currentLog = logsWithErrors[i].second;
      if (!currentLog.hasErrors())
        continue;

      QString currentMessage = "For value " + QString::number(logsWithErrors[i].first + 1) + ":\n";

      const std::vector<smtk::io::Logger::Record> recs = currentLog.records();
      for (auto recIt = recs.begin(); recIt != recs.end(); ++recIt)
      {
        currentMessage += recIt->message.c_str();
        if (recIt != --recs.end())
          currentMessage += "\n";
      }

      toolTipText += currentMessage;

      if (i < static_cast<int>(logsWithErrors.size()) - 1)
        toolTipText += "\n\n";
    }

    if (!warningText.isEmpty())
    {
      toolTipText += "\n\n";
      toolTipText += warningText;
    }

    this->showExpressionResultWidgets(false, QString("Evaluation failed"), toolTipText);

    if (showMessageBox)
    {
      QMessageBox::critical(
        m_internals->m_expressionResultLineEdit, "Evaluation Error", toolTipText);
    }
  }
  else
  {
    this->showExpressionResultWidgets(true, valueStrings.join(", "), warningText);
  }
}

void qtInputsItem::hideExpressionResultWidgets()
{
  m_internals->m_expressionEqualsLabel->setVisible(false);
  m_internals->m_expressionResultLineEdit->setVisible(false);
}

void qtInputsItem::showExpressionResultWidgets(
  bool success,
  const QString& text,
  const QString& tooltip)
{
  if (success)
  {
    if (tooltip.isEmpty())
    {
      uiManager()->setWidgetColorToNormal(m_internals->m_expressionResultLineEdit);
    }
    else
    {
      QPalette pal = m_internals->m_expressionResultLineEdit->palette();
      pal.setColor(QPalette::Base, qtUIManager::contrastWithText(QColor(100, 149, 237)));
      m_internals->m_expressionResultLineEdit->setPalette(pal);
    }
  }
  else
  {
    uiManager()->setWidgetColorToInvalid(m_internals->m_expressionResultLineEdit);
  }
  m_internals->m_expressionResultLineEdit->setToolTip(tooltip);

  // Add units to the text if definition has units string recognized by units system
  QString displayText = text;
  auto item = m_itemInfo.itemAs<ValueItem>();
  auto def =
    std::dynamic_pointer_cast<const smtk::attribute::ValueItemDefinition>(item->definition());
  if (!def->units().empty())
  {
    auto unitsSystem = item->attribute()->attributeResource()->unitsSystem();
    bool parsed = false;
    unitsSystem->unit(def->units(), &parsed);
    if (parsed)
    {
      displayText = QString("%1 %2").arg(text).arg(def->units().c_str());
    }
  }
  m_internals->m_expressionResultLineEdit->setText(displayText);

  m_internals->m_expressionResultLineEdit->setVisible(true);
  m_internals->m_expressionEqualsLabel->setVisible(true);
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
  QBoxLayout* childLayout = nullptr;
  if (item->isDiscrete())
  {
    if (m_internals->VectorItemOrient == Qt::Vertical)
    {
      childLayout = new QVBoxLayout;
    }
    else
    {
      childLayout = new QHBoxLayout;
      childLayout->setContentsMargins(4, 0, 0, 0);
    }

    childLayout->setObjectName(QString("childLayout%1").arg(i));
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
  editorLayout->setObjectName(QString("editorLayout%1").arg(i));
  editorLayout->setMargin(0);
  editorLayout->setSpacing(3);
  if (item->isExtensible())
  {
    QToolButton* minusButton = new QToolButton(m_widget);
    minusButton->setObjectName(QString("minusButton%1").arg(i));
    QString iconName(":/icons/attribute/minus.png");
    minusButton->setFixedSize(QSize(12, 12));
    minusButton->setIcon(QIcon(iconName));
    minusButton->setSizePolicy(sizeFixedPolicy);
    minusButton->setToolTip("Remove value");
    minusButton->setFocusPolicy(Qt::ClickFocus);
    editorLayout->addWidget(minusButton);
    connect(minusButton, SIGNAL(clicked()), this, SLOT(onRemoveValue()));
    QPair<QPointer<QLayout>, QPointer<QWidget>> pair;
    pair.first = editorLayout;
    pair.second = editBox;
    m_internals->ExtensibleMap[minusButton] = pair;
    m_internals->MinusButtonIndices.push_back(minusButton);
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
  if (m_internals->VectorItemOrient == Qt::Vertical || item->isExtensible())
  {
    // The "Add New Value" button is in first row, so take that into account
    int row = item->isExtensible() ? i + 1 : i;
    m_internals->EntryLayout->addLayout(editorLayout, row, 1);
    // there could be conditional children, so we need another layout
    // so that the combobox will stay TOP-left when there are multiple
    // combo boxes.
    if (item->isDiscrete() && childLayout)
    {
      int row = m_internals->VectorItemOrient == Qt::Vertical ? 2 : 1;
      int col = m_internals->VectorItemOrient == Qt::Vertical ? 1 : 2;
      m_internals->EntryLayout->addLayout(childLayout, row, col);
    }
  }
  else // going horizontal
  {
    int column = 2 * i;
    m_internals->EntryLayout->addLayout(editorLayout, 0, column + 1);
    if (item->isDiscrete() && childLayout)
    {
      m_internals->EntryLayout->addLayout(childLayout, 0, column + 2);
    }
  }

  m_internals->ChildrenMap[editBox] = childLayout;
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
    if (!m_internals->AddItemButton)
    {
      QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      m_internals->AddItemButton = new QToolButton(m_widget);
      m_internals->AddItemButton->setObjectName(QString("AddItemButton"));
      QString iconName(":/icons/attribute/plus.png");
      m_internals->AddItemButton->setText("Add New Value");
      m_internals->AddItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

      //      m_internals->AddItemButton->setFixedSize(QSize(12, 12));
      m_internals->AddItemButton->setIcon(QIcon(iconName));
      m_internals->AddItemButton->setSizePolicy(sizeFixedPolicy);
      connect(m_internals->AddItemButton, SIGNAL(clicked()), this, SLOT(onAddNewValue()));
      m_internals->EntryLayout->addWidget(m_internals->AddItemButton, 0, 1);
    }
  }

  for (int i = 0; i < n; i++)
  {
    this->addInputEditor(i);
  }
}

QFrame* qtInputsItem::createLabelFrame(
  const smtk::attribute::ValueItem* vitem,
  const smtk::attribute::ValueItemDefinition* vitemDef)
{
  smtk::attribute::ValueItemPtr dataObj = m_itemInfo.itemAs<ValueItem>();
  auto itemDef = dataObj->definitionAs<ValueItemDefinition>();
  auto* iview = m_itemInfo.baseView();
  // Lets create the label and proper decorations
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto* labelFrame = new QFrame();
  labelFrame->setObjectName("labelFrame");
  QHBoxLayout* labelLayout = new QHBoxLayout(labelFrame);
  labelLayout->setObjectName("labelLayout");
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;

  // Note that the definition could be optional but the item maybe forced
  // to be required.  We need to still create the check box in case
  // the item's force required state is changed
  if (vitemDef->isOptional() && (m_internals->OptionalCheck == nullptr))
  {
    m_internals->OptionalCheck = new QCheckBox(m_itemInfo.parentWidget());
    m_internals->OptionalCheck->setObjectName("OptionalCheck");
    m_internals->OptionalCheck->setChecked(vitem->localEnabledState());
    m_internals->OptionalCheck->setText(" ");
    m_internals->OptionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = m_internals->OptionalCheck->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(
      m_internals->OptionalCheck, SIGNAL(stateChanged(int)), this, SLOT(setOutputOptional(int)));
    labelLayout->addWidget(m_internals->OptionalCheck);
    if (!vitem->isOptional())
    {
      m_internals->OptionalCheck->setVisible(false);
      this->setOutputOptional(1);
    }
  }

  QString labelText = vitem->label().c_str();
  QLabel* label = new QLabel(labelText, m_widget);
  label->setObjectName("label");
  label->setSizePolicy(sizeFixedPolicy);
  if (iview)
  {
    int requiredLen = m_itemInfo.uiManager()->getWidthOfText(
      vitem->label(), m_itemInfo.uiManager()->advancedFont());
    int labLen = iview->fixedLabelWidth();
    if ((requiredLen / 2) > labLen)
    {
      labLen = requiredLen;
    }
    label->setFixedWidth(labLen - padding);
  }
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  //  qtOverlayFilter *filter = new qtOverlayFilter(this);
  //  label->installEventFilter(filter);

  // add in BriefDescription as tooltip if available
  const std::string& strBriefDescription = vitemDef->briefDescription();
  if (!strBriefDescription.empty())
  {
    label->setToolTip(strBriefDescription.c_str());
  }

  if (!vitemDef->units().empty())
  {
    // Check if we should units to the label
    std::string option = "LineEdit"; // default behavior
    m_itemInfo.component().attribute("Option", option);
    bool addUnitsLabel = (option == "LineEdit");
    if (addUnitsLabel && !vitemDef->isDiscrete())
    {
      // Check if units are "valid"
      auto unitsSystem = vitem->attribute()->attributeResource()->unitsSystem();
      if (unitsSystem)
      {
        bool unitsParsed = false;
        units::Unit defUnit = unitsSystem->unit(vitemDef->units(), &unitsParsed);
        addUnitsLabel = addUnitsLabel && (!unitsParsed);
      }
    }
    if (addUnitsLabel)
    {
      QString unitText = label->text();
      unitText.append(" (").append(vitemDef->units().c_str()).append(")");
      label->setText(unitText);
    }
  }
  if (vitemDef->advanceLevel() && m_itemInfo.baseView())
  {
    label->setFont(m_itemInfo.uiManager()->advancedFont());
  }
  labelLayout->addWidget(label);
  m_internals->theLabel = label;
  if (label->text().trimmed().isEmpty())
  {
    this->setLabelVisible(false);
  }
  if (vitem->allowsExpressions())
  {
    m_internals->m_expressionButton = new QToolButton(m_widget);
    m_internals->m_expressionButton->setObjectName("expressionButton");
    m_internals->m_expressionButton->setCheckable(true);
    QString resourceName(":/icons/attribute/function.png");
    m_internals->m_expressionButton->setIconSize(QSize(13, 13));
    m_internals->m_expressionButton->setIcon(QIcon(resourceName));
    m_internals->m_expressionButton->setSizePolicy(sizeFixedPolicy);
    m_internals->m_expressionButton->setToolTip(
      "Switch between a constant value or function instance");
    m_internals->m_expressionButton->setFocusPolicy(Qt::ClickFocus);
    QObject::connect(
      m_internals->m_expressionButton,
      SIGNAL(toggled(bool)),
      this,
      SLOT(displayExpressionWidget(bool)));
    labelLayout->addWidget(m_internals->m_expressionButton);
  }

  return labelFrame;
}

void qtInputsItem::updateUI()
{
  smtk::attribute::ValueItemPtr dataObj = m_itemInfo.itemAs<ValueItem>();
  auto itemDef = dataObj->definitionAs<ValueItemDefinition>();
  auto* iview = m_itemInfo.baseView();
  if (iview && !iview->displayItem(dataObj))
  {
    return;
  }
  m_internals->m_editors.clear();

  m_widget = new QFrame(this->parentWidget());
  m_widget->setObjectName(dataObj->name().c_str());
  if (this->isReadOnly())
  {
    m_widget->setEnabled(false);
  }
  auto* mainlayout = new QHBoxLayout(m_widget);
  mainlayout->setMargin(0);
  mainlayout->setSpacing(0);
  mainlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  m_internals->m_dataFrame = new QFrame(m_widget);
  m_internals->m_dataFrame->setObjectName("dataFrame");

  // Add Label Information
  mainlayout->addWidget(this->createLabelFrame(dataObj.get(), itemDef.get()));

  // Add Data Section
  auto* dataLayout = new QVBoxLayout(m_internals->m_dataFrame);
  dataLayout->setObjectName("dataLayout");
  dataLayout->setMargin(0);
  dataLayout->setSpacing(0);
  dataLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  // This section will either display values or the expression (if this item supports them)
  m_internals->m_valuesFrame = new QFrame(m_internals->m_dataFrame);
  m_internals->m_valuesFrame->setObjectName("valuesFrame");

  m_internals->EntryLayout = new QGridLayout(m_internals->m_valuesFrame);
  m_internals->EntryLayout->setObjectName("EntryLayout");
  m_internals->EntryLayout->setMargin(0);
  m_internals->EntryLayout->setSpacing(0);
  m_internals->EntryLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  this->loadInputValues();
  dataLayout->addWidget(m_internals->m_valuesFrame);

  // Does this item allow expressions?
  if (dataObj->allowsExpressions())
  {
    m_internals->m_expressionFrame = this->createExpressionRefFrame();
    dataLayout->addWidget(m_internals->m_expressionFrame);
  }

  mainlayout->addWidget(m_internals->m_dataFrame);

  // Lets see if this item is always suppose to be an expression
  if (m_internals->m_expressionButton)
  {
    bool expressionOnly = m_itemInfo.component().attributeAsBool("ExpressionOnly");
    m_internals->m_expressionButton->setChecked(dataObj->isExpression() || expressionOnly);
    this->displayExpressionWidget(dataObj->isExpression() || expressionOnly);
    m_internals->m_expressionButton->setEnabled(!expressionOnly);
    if (expressionOnly)
    {
      m_internals->m_usingExpression = true;
    }
  }

  if (this->parentWidget() && this->parentWidget()->layout())
  {
    this->parentWidget()->layout()->addWidget(m_widget);
  }
  if (dataObj->isOptional())
  {
    this->setOutputOptional(dataObj->localEnabledState() ? 1 : 0);
  }
}

void qtInputsItem::setOutputOptional(int state)
{
  smtk::attribute::ValueItemPtr item = m_itemInfo.itemAs<ValueItem>();
  if (!item)
  {
    return;
  }
  bool enable = state != 0;
  // This controls the visibility of the data frame and the expression button (if one exists)
  m_internals->m_dataFrame->setVisible(enable);
  if (m_internals->m_expressionButton)
  {
    m_internals->m_expressionButton->setVisible(enable);
  }
  //  m_internals->EntryFrame->setEnabled(enable);
  if (!(item->forceRequired() || (enable == item->localEnabledState())))
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
    //      m_internals->EntryFrame->layout());
    this->addInputEditor(static_cast<int>(item->numberOfValues()) - 1);
  }
  Q_EMIT this->modified();
}

void qtInputsItem::onRemoveValue()
{
  QToolButton* const minusButton = qobject_cast<QToolButton*>(QObject::sender());
  if (!minusButton || !m_internals->ExtensibleMap.contains(minusButton))
  {
    return;
  }

  int gIdx = m_internals->MinusButtonIndices.indexOf(
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
  Q_EMIT this->modified();
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
  m_internals->AddItemButton->setEnabled(!maxReached);

  bool minReached = (item->numberOfRequiredValues() > 0) &&
    (item->numberOfRequiredValues() == item->numberOfValues());
  Q_FOREACH (QToolButton* tButton, m_internals->ExtensibleMap.keys())
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
    Q_FOREACH (QToolButton* tButton, m_internals->ExtensibleMap.keys())
    {
      // will delete later from m_internals->ChildrenMap
      //      delete m_internals->ExtensibleMap.value(tButton).second;
      delete m_internals->ExtensibleMap.value(tButton).first;
      delete tButton;
    }
    m_internals->ExtensibleMap.clear();
    m_internals->MinusButtonIndices.clear();
  }

  Q_FOREACH (QWidget* cwidget, m_internals->ChildrenMap.keys())
  {
    QLayout* childLayout = m_internals->ChildrenMap.value(cwidget);
    if (childLayout)
    {
      QLayoutItem* child;
      while ((child = childLayout->takeAt(0)) != nullptr)
      {
        delete child;
      }
      delete childLayout;
    }
    delete cwidget;
  }
  m_internals->ChildrenMap.clear();
  m_internals->DiscreteEditors.clear();
}

QWidget* qtInputsItem::createInputWidget(int elementIdx, QLayout* childLayout)
{
  smtk::attribute::ValueItemPtr item = m_itemInfo.itemAs<ValueItem>();
  if (!item)
  {
    return nullptr;
  }
  while (m_internals->m_editors.count() <= elementIdx)
  {
    m_internals->m_editors << nullptr;
  }

  if (item->isDiscrete())
  {
    auto* editor = new qtDiscreteValueEditor(this, elementIdx, childLayout);
    editor->setObjectName(QString("editor%1").arg(elementIdx));
    QObject::connect(editor, SIGNAL(widgetSizeChanged()), this, SIGNAL(widgetSizeChanged()));
    // editor->setUseSelectionManager(m_useSelectionManager);
    m_internals->DiscreteEditors.append(editor);
    m_internals->m_editors[elementIdx] = editor->lastEditingWidget();
    QObject::connect(
      editor,
      &qtDiscreteValueEditor::editingWidgetChanged,
      this,
      &qtInputsItem::editingWidgetChanged);
    return editor;
  }

  // (else)
  QWidget* editorWidget = this->createEditBox(elementIdx, m_widget);
  m_internals->m_editors[elementIdx] = editorWidget;
  return editorWidget;
}

QFrame* qtInputsItem::createExpressionRefFrame()
{
  auto* frame = new QFrame();
  frame->setObjectName("frame");
  QHBoxLayout* expressionLayout = new QHBoxLayout(frame);
  expressionLayout->setObjectName("expressionLayout");
  expressionLayout->setMargin(0);
  expressionLayout->setSpacing(0);
  expressionLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // create combobox for expression reference
  m_internals->m_expressionCombo = new QComboBox(frame);
  m_internals->m_expressionCombo->setObjectName("expressionCombo");
  QObject::connect(
    m_internals->m_expressionCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged),
    this,
    &qtInputsItem::onExpressionReferenceChanged,
    Qt::QueuedConnection);
  m_internals->m_expressionCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  m_internals->m_expressionEqualsLabel = new QLabel("=", frame);
  m_internals->m_expressionEqualsLabel->setObjectName("expressionEqualsLabel");
  m_internals->m_expressionEqualsLabel->setVisible(false);

  m_internals->m_expressionResultLineEdit = new QLineEdit(frame);
  m_internals->m_expressionResultLineEdit->setObjectName("expressionResultLineEdit");
  m_internals->m_expressionResultLineEdit->setVisible(false);
  m_internals->m_expressionResultLineEdit->setReadOnly(true);

  expressionLayout->addWidget(m_internals->m_expressionCombo);
  expressionLayout->addWidget(m_internals->m_expressionEqualsLabel);
  expressionLayout->addWidget(m_internals->m_expressionResultLineEdit);

  expressionLayout->setContentsMargins(0, 0, 0, 0);
  return frame;
}

void qtInputsItem::displayExpressionWidget(bool checkstate)
{
  if (m_internals->m_expressionButton == nullptr)
  {
    return;
  }

  auto inputitem = m_itemInfo.itemAs<ValueItem>();
  if (!inputitem)
  {
    return;
  }
  m_internals->m_usingExpression = checkstate;

  ResourcePtr sourceAttResource = inputitem->attribute()->attributeResource();

  if (checkstate)
  {
    m_internals->m_expressionCombo->blockSignals(true);
    m_internals->m_expressionCombo->clear();
    auto valItemDef = inputitem->definitionAs<ValueItemDefinition>();
    // Lets find the attribute resource that contains the expression information
    ResourcePtr lAttResource = smtk::attribute::utility::findResourceContainingDefinition(
      valItemDef->expressionType(), sourceAttResource, this->uiManager()->resourceManager());
    if (lAttResource == nullptr)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        " Could not find any AttributeResource containing Expressions of Type: "
          << valItemDef->expressionType());
      return;
    }
    smtk::attribute::DefinitionPtr attDef =
      lAttResource->findDefinition(valItemDef->expressionType());
    QStringList attNames;

    int setIndex = 0;

    //Lets build the list of possible expressions
    if (attDef)
    {
      std::vector<smtk::attribute::AttributePtr> result;
      lAttResource->findAttributes(attDef, result);
      std::vector<smtk::attribute::AttributePtr>::iterator it;
      for (it = result.begin(); it != result.end(); ++it)
      {
        attNames.push_back((*it)->name().c_str());
      }
      attNames.sort();
      // Now add Please Select and Create Options
      attNames.insert(0, "Please Select");
      attNames.insert(1, "Create...");
      m_internals->m_expressionCombo->addItems(attNames);
    }

    // Can we find the item's current expression?
    if (inputitem->isExpression())
    {
      smtk::attribute::AttributePtr att = inputitem->expression();
      if (att != nullptr)
      {
        setIndex = attNames.indexOf(att->name().c_str());

        if (att->canEvaluate())
        {
          updateExpressionRefWidgetForEvaluation(inputitem, false);
        }
        else
        {
          hideExpressionResultWidgets();
        }
      }
    }
    // If we have not found a valid expression and we have the name of the
    // expression that was used previously lets try that
    if ((setIndex == 0) && (m_internals->m_lastExpression != ""))
    {
      AttributePtr attPtr =
        lAttResource->findAttribute(m_internals->m_lastExpression.toStdString());
      if (attPtr)
      {
        setIndex = attNames.indexOf(m_internals->m_lastExpression);
        inputitem->setExpression(attPtr);

        if (attPtr->canEvaluate())
        {
          updateExpressionRefWidgetForEvaluation(inputitem, false);
        }
        else
        {
          hideExpressionResultWidgets();
        }

        Q_EMIT this->modified();
      }
    }

    // If item is set but we could not find the expression - lets clear it and send a modified
    if (inputitem->isExpression() && (setIndex == 0))
    {
      inputitem->setExpression(nullptr);
      Q_EMIT this->modified();
    }
    m_internals->m_expressionCombo->setCurrentIndex(setIndex);
    m_internals->m_expressionCombo->blockSignals(false);
  }
  else
  {
    // OK - so now we need to deal with going from an expression to
    // to a constant - Simply clear the expression which will force the item
    // to revert back to using values
    if (inputitem->isExpression())
    {
      // Lets save the current name of the expression attribute in case
      // the user simply wants to switch back
      m_internals->m_lastExpression = inputitem->expression()->name().c_str();
      inputitem->setExpression(nullptr);

      hideExpressionResultWidgets();

      // Since the item had an expression set (which influences the isSet method)
      // we should force a reload of the item's non-expression values to make sure
      // they are correct.
      this->clearChildWidgets();
      this->loadInputValues();
      Q_EMIT this->modified();
    }
  }

  bool widgetChanged = checkstate == m_internals->m_valuesFrame->isVisible();

  m_internals->m_valuesFrame->setVisible(!checkstate);
  m_internals->m_expressionFrame->setVisible(checkstate);

  if (QObject::sender() && widgetChanged)
  {
    QTimer::singleShot(0, [this]() { Q_EMIT this->editingWidgetChanged(); });
  }
}

void qtInputsItem::onExpressionReferenceChanged()
{
  if (!m_internals->m_expressionCombo)
  {
    return;
  }
  int curIdx = m_internals->m_expressionCombo->currentIndex();
  auto inputitem = m_itemInfo.itemAs<ValueItem>();
  if (!inputitem)
  {
    return;
  }
  smtk::attribute::ResourcePtr sourceAttResource = inputitem->attribute()->attributeResource();
  auto valItemDef = inputitem->definitionAs<ValueItemDefinition>();
  // Lets find the attribute resource that contains the expression information
  ResourcePtr lAttResource = smtk::attribute::utility::findResourceContainingDefinition(
    valItemDef->expressionType(), sourceAttResource, this->uiManager()->resourceManager());

  smtk::attribute::ComponentItemPtr item = inputitem->expressionReference();
  if (!item)
  {
    return;
  }

  if (curIdx == 0)
  {
    item->unset();

    hideExpressionResultWidgets();
  }
  else if (curIdx == 1)
  {
    smtk::attribute::DefinitionPtr attDef = valItemDef->expressionDefinition(lAttResource);
    smtk::attribute::AttributePtr newAtt = lAttResource->createAttribute(attDef->type());
    auto* editor =
      new smtk::extension::qtAttributeEditorDialog(newAtt, m_itemInfo.uiManager(), m_widget);
    auto status = editor->exec();
    QStringList itemsInComboBox;
    if (status == QDialog::Rejected)
    {
      lAttResource->removeAttribute(newAtt);
    }
    else
    {
      // The user has created a new expression so add it
      // to the list of expression names and set the item to use it
      inputitem->setExpression(newAtt);
      itemsInComboBox.append(newAtt->name().c_str());

      // Signal that a new attribute was created - since this instance is not
      // observing the Operation Manager we don't need to set the source parameter
      auto signalOp = this->uiManager()->operationManager()->create<smtk::attribute::Signal>();
      signalOp->parameters()->findComponent("created")->appendValue(newAtt);
      signalOp->operate();
    }
    for (int index = 2; index < m_internals->m_expressionCombo->count(); index++)
    {
      itemsInComboBox << m_internals->m_expressionCombo->itemText(index);
    }
    itemsInComboBox.sort();
    // Now add Please Select and Create Options
    itemsInComboBox.insert(0, "Please Select");
    itemsInComboBox.insert(1, "Create...");
    m_internals->m_expressionCombo->blockSignals(true);
    m_internals->m_expressionCombo->clear();
    m_internals->m_expressionCombo->addItems(itemsInComboBox);
    auto expressionAtt = inputitem->expression();
    if (expressionAtt == nullptr)
    {
      m_internals->m_expressionCombo->setCurrentIndex(0);
    }
    else
    {
      auto index = itemsInComboBox.indexOf(expressionAtt->name().c_str());
      m_internals->m_expressionCombo->setCurrentIndex(index);
    }
    m_internals->m_expressionCombo->blockSignals(false);

    hideExpressionResultWidgets();
  }
  else
  {
    AttributePtr attPtr =
      lAttResource->findAttribute(m_internals->m_expressionCombo->currentText().toStdString());
    if (inputitem->isSet() && attPtr == inputitem->expression())
    {
      hideExpressionResultWidgets();
      return; // nothing to do
    }

    if (attPtr)
    {
      inputitem->setExpression(attPtr);

      if (attPtr->canEvaluate())
      {
        updateExpressionRefWidgetForEvaluation(inputitem, false);
      }
      else
      {
        hideExpressionResultWidgets();
      }
    }
  }

  auto currentExpression = inputitem->expression();
  if (currentExpression == nullptr)
  {
    m_internals->m_lastExpression = "";
  }
  else
  {
    m_internals->m_lastExpression = currentExpression->name().c_str();
  }

  auto* iview = m_itemInfo.baseView();
  if (iview)
  {
    iview->valueChanged(inputitem->shared_from_this());
  }
  Q_EMIT this->modified();
}

QWidget* qtInputsItem::createDoubleWidget(
  int elementIdx,
  ValueItemPtr vitem,
  QWidget* pWidget,
  QString& tooltip)
{
  auto dDef = vitem->definitionAs<DoubleItemDefinition>();
  auto ditem = dynamic_pointer_cast<DoubleItem>(vitem);
  double minVal, maxVal;

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
    int defaultIdx = static_cast<int>(dDef->defaultValues().size()) <= elementIdx ? 0 : elementIdx;
    std::string defValString = dDef->defaultValueAsString(defaultIdx);
    if (!tooltip.isEmpty())
    {
      tooltip.append("; ");
    }
    tooltip.append("Default: ").append(defValString.c_str());
  }

  // What type of option are we suppose to use
  std::string option = "LineEdit";
  // By default we should be using a line edit, lets see if
  // we were told what to use
  m_itemInfo.component().attribute("Option", option);

  if (option == "LineEdit")
  {
    QWidget* editorWidget = nullptr;

    bool expressionOnly = m_itemInfo.component().attributeAsBool("ExpressionOnly");
    if (!expressionOnly)
    {
      // First check if we should use units-aware editor (qtDoubleUnitsLineEdit)
      auto* unitEditor = qtDoubleUnitsLineEdit::checkAndCreate(this, tooltip);
      if (unitEditor != nullptr)
      {
        editorWidget = unitEditor;
        // Prep the widget
        unitEditor->setObjectName(QString("editBox%1").arg(elementIdx));
        std::string notation("Mixed");
        m_itemInfo.component().attribute("Notation", notation);
        if (notation == "Fixed")
        {
          unitEditor->setNotation(qtDoubleUnitsLineEdit::FixedNotation);
        }
        else if (notation == "Scientific")
        {
          unitEditor->setNotation(qtDoubleUnitsLineEdit::ScientificNotation);
        }
        int precision = 0;
        m_itemInfo.component().attributeAsInt("Precision", precision);
        if (precision > 0)
        {
          unitEditor->setPrecision(precision);
        }
        // Set the value
        if (vitem->isSet(elementIdx))
        {
          QSignalBlocker blocker(unitEditor);
          std::string valueAsString = vitem->valueAsString(elementIdx);
          unitEditor->setText(valueAsString.c_str());
        }
        // Added the converted value to the tool-tip
        if (ditem->isSet(elementIdx))
        {
          QString val;
          val.setNum(ditem->value(elementIdx));
          if (!tooltip.isEmpty())
          {
            tooltip.append("; ");
          }
          tooltip.append("Val: ").append(val).append(" ").append(dDef->units().c_str());
        }
      }
    }

    // If units editor not created, use qtDoubleLineEdit
    if (editorWidget == nullptr)
    {
      auto* editBox = new qtDoubleLineEdit(pWidget);
      editBox->setObjectName(QString("editBox%1").arg(elementIdx));

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
      validator->setObjectName(QString("validator%1").arg(elementIdx));

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
        if (m_internals->m_editPrecision > 0)
        {
          editBox->setText(
            QString::number(ditem->value(elementIdx), 'f', m_internals->m_editPrecision));
        }
        else
        {
          // We need to split the value string into 2 parts and only display the value
          // not the units
          std::string valStr, unitsStr;
          DoubleItemDefinition::splitStringStartingDouble(
            vitem->valueAsString(elementIdx), valStr, unitsStr);
          editBox->setText(valStr.c_str());
        }
      }

      editorWidget = static_cast<QWidget*>(editBox);
    }

    return editorWidget;
  }

  if (option == "SpinBox")
  {
    QDoubleSpinBox* spinbox = new QDoubleSpinBox(pWidget);
    spinbox->setObjectName(QString("spinbox%1").arg(elementIdx));
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
  int elementIdx,
  ValueItemPtr vitem,
  QWidget* pWidget,
  QString& tooltip)
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
    int defaultIdx = static_cast<int>(iDef->defaultValues().size()) <= elementIdx ? 0 : elementIdx;
    defVal = iDef->defaultValue(defaultIdx);
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
    editBox->setObjectName(QString("editBox%1").arg(elementIdx));
    qtIntValidator* validator = new qtIntValidator(this, elementIdx, editBox, pWidget);
    validator->setObjectName(QString("validator%1").arg(elementIdx));

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
    spinbox->setObjectName(QString("spinbox%1").arg(elementIdx));
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
    return nullptr;
  }

  QWidget* inputWidget = nullptr;
  QVariant vdata(elementIdx);
  QString tooltip;

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
        textEdit->setObjectName(QString("textEdit%1").arg(elementIdx));
        textEdit->setPlainText(valText);
        QObject::connect(textEdit, SIGNAL(textChanged()), this, SLOT(onTextEditChanged()));
        inputWidget = textEdit;
      }
      else
      {
        QLineEdit* lineEdit = new QLineEdit(pWidget);
        lineEdit->setObjectName(QString("lineEdit%1").arg(elementIdx));
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
        int defaultIdx =
          static_cast<int>(sDef->defaultValues().size()) <= elementIdx ? 0 : elementIdx;
        tooltip.append("Default: ").append(sDef->defaultValue(defaultIdx).c_str());
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
    return nullptr;
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
  QPointer<qtInputsItem> self(this);
  if (QLineEdit* const lineEdit = qobject_cast<QLineEdit*>(inputWidget))
  {
    QObject::connect(
      lineEdit,
      SIGNAL(textChanged(const QString&)),
      this,
      SLOT(onLineEditChanged()),
      Qt::QueuedConnection);
    lineEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(
      lineEdit, &QLineEdit::customContextMenuRequested, this, [self, elementIdx](const QPoint& pt) {
        if (!self)
        {
          return;
        }
        self->showContextMenu(pt, elementIdx);
      });
    if (auto* const unitsEdit = qobject_cast<qtDoubleUnitsLineEdit*>(lineEdit))
    {
      QObject::connect(
        unitsEdit,
        SIGNAL(editingCompleted(QObject*)),
        this,
        SLOT(onInputValueChanged(QObject*)),
        Qt::QueuedConnection);
    }
    else
    {
      QObject::connect(
        lineEdit,
        SIGNAL(editingFinished()),
        this,
        SLOT(onLineEditFinished()),
        Qt::QueuedConnection);
    }
  }
  else if (QTextEdit* const textEdit = qobject_cast<QTextEdit*>(inputWidget))
  {
    textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(
      textEdit, &QTextEdit::customContextMenuRequested, this, [self, elementIdx](const QPoint& pt) {
        if (!self)
        {
          return;
        }
        self->showContextMenu(pt, elementIdx);
      });
  }
  else if (auto* const spinbox = qobject_cast<QAbstractSpinBox*>(inputWidget))
  {
    // spin boxes don't provide their context menus - so we add an action after it's shown
    spinbox->installEventFilter(this);
  }

  return inputWidget;
}

void qtInputsItem::showContextMenu(const QPoint& pt, int elementIdx)
{
  QLineEdit* const lineEdit = qobject_cast<QLineEdit*>(QObject::sender());
  QTextEdit* const textEdit = qobject_cast<QTextEdit*>(QObject::sender());
  auto item = this->m_itemInfo.itemAs<ValueItem>();
  if (!(textEdit || lineEdit) || !item)
  {
    return;
  }
  QMenu* menu =
    (lineEdit ? lineEdit->createStandardContextMenu() : textEdit->createStandardContextMenu());
  auto* resetDefault = new QAction("Reset to Default");
  if (item->hasDefault())
  {
    QPointer<qtInputsItem> self(this);
    QObject::connect(
      resetDefault, &QAction::triggered, this, [self, elementIdx, lineEdit, textEdit]() {
        if (!self)
        {
          return;
        }
        auto item = self->m_itemInfo.itemAs<ValueItem>();
        if (item)
        {
          item->setToDefault(elementIdx);
          if (lineEdit)
          {
            lineEdit->setText(item->valueAsString(elementIdx).c_str());
          }
          else if (textEdit)
          {
            textEdit->setText(item->valueAsString(elementIdx).c_str());
          }
          Q_EMIT self->modified();
        }
      });
  }
  else
  {
    resetDefault->setEnabled(false);
  }
  menu->addAction(resetDefault);
  menu->exec(lineEdit ? lineEdit->mapToGlobal(pt) : textEdit->mapToGlobal(pt));
  delete menu;
}

bool qtInputsItem::eventFilter(QObject* filterObj, QEvent* ev)
{
  auto* spinbox = qobject_cast<QAbstractSpinBox*>(filterObj);
  if (ev->type() == QEvent::ContextMenu && spinbox)
  {
    int elementIdx = spinbox->property("ElementIndex").toInt();
    QPointer<qtInputsItem> self(this);
    // we want the default context menu, we just add something to it.
    QTimer::singleShot(0, this, [self, this, elementIdx, spinbox]() {
      if (!self)
      {
        return;
      }
      // find the menu that was just displayed by name.
      for (QWidget* w : QApplication::topLevelWidgets())
      {
        auto* contextMenu = qobject_cast<QMenu*>(w);
        if (contextMenu && w->objectName() == "qt_edit_menu")
        {
          // create a reset to default action and append to the end.
          auto* resetDefault = new QAction("Reset to Default");
          auto item = self->m_itemInfo.itemAs<ValueItem>();
          if (item && item->hasDefault())
          {
            QObject::connect(
              resetDefault, &QAction::triggered, this, [self, elementIdx, spinbox]() {
                if (!self)
                {
                  return;
                }
                auto item = self->m_itemInfo.itemAs<IntItem>();
                auto* intBox = qobject_cast<QSpinBox*>(spinbox);
                if (item && intBox)
                {
                  item->setToDefault(elementIdx);
                  intBox->setValue(item->value(elementIdx));
                  Q_EMIT self->modified();
                  return;
                }
                auto ditem = self->m_itemInfo.itemAs<DoubleItem>();
                auto* dblBox = qobject_cast<QDoubleSpinBox*>(spinbox);
                if (ditem && dblBox)
                {
                  ditem->setToDefault(elementIdx);
                  dblBox->setValue(ditem->value(elementIdx));
                  Q_EMIT self->modified();
                  return;
                }
              });
          }
          else
          {
            resetDefault->setEnabled(false);
          }
          contextMenu->addAction(resetDefault);
        }
      }
    });
  }
  return QObject::eventFilter(filterObj, ev);
}

void qtInputsItem::onTextEditChanged()
{
  this->onInputValueChanged(QObject::sender());
}

void qtInputsItem::onLineEditChanged()
{
  // Here we only handle changes when this is invoked from setText()
  // which is normally used programmatically, and the setText() will have
  // modified flag reset to false;
  QLineEdit* const editBox = qobject_cast<QLineEdit*>(QObject::sender());
  if (!editBox)
  {
    return;
  }

  else if (!editBox->isModified())
  {
    // If this is not from setText(), ignore it. We are using editingFinished
    // signal to handle others.
    this->onInputValueChanged(editBox);
  }
}

void qtInputsItem::onLineEditFinished()
{
  QObject* sender = QObject::sender();
  this->onInputValueChanged(sender);
}

void qtInputsItem::doubleValueChanged(double newVal)
{
  auto* obj = QObject::sender();
  auto* senderWidget = qobject_cast<QWidget*>(obj);
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
  if ((ditem->isExpression() || !ditem->isSet(elementIdx)) || val != newVal)
  {
    ditem->setValue(elementIdx, newVal);
    valChanged = true;
  }
  // Lets determine if the item is set to the default value -
  isDefault = ditem->isUsingDefault(elementIdx);
  isInvalid = !ditem->isSet(elementIdx);
  auto* iview = m_itemInfo.baseView();
  if (valChanged)
  {
    if (iview)
    {
      iview->valueChanged(ditem);
    }
    Q_EMIT this->modified();
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
  auto* obj = QObject::sender();
  auto* senderWidget = qobject_cast<QWidget*>(obj);
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
  if ((iitem->isExpression() || !iitem->isSet(elementIdx)) || val != newVal)
  {
    iitem->setValue(elementIdx, newVal);
    valChanged = true;
  }
  // Lets determine if the item is set to the default value -
  isDefault = iitem->isUsingDefault(elementIdx);
  isInvalid = !iitem->isSet(elementIdx);
  auto* iview = m_itemInfo.baseView();
  if (valChanged)
  {
    if (iview)
    {
      iview->valueChanged(iitem);
    }
    Q_EMIT this->modified();
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
  if (editBox != nullptr)
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

  qtDoubleUnitsLineEdit* valueUnitsBox = qobject_cast<qtDoubleUnitsLineEdit*>(obj);

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
      if (valueUnitsBox != nullptr)
      {
        // Do we have a new value?
        if (ditem->valueAsString(elementIdx) != editBox->text().toStdString())
        {
          QString newToolTip = valueUnitsBox->baseToolTipText();
          if (!ditem->setValueFromString(elementIdx, editBox->text().toStdString()))
          {
            this->unsetValue(elementIdx); // editor's contents are invalid
          }
          else
          {
            QString val;
            val.setNum(ditem->value(elementIdx));
            auto dDef = ditem->definitionAs<DoubleItemDefinition>();
            if (!newToolTip.isEmpty())
            {
              newToolTip.append("; ");
            }
            newToolTip.append("Val: ").append(val).append(" ").append(dDef->units().c_str());
          }
          valueUnitsBox->setToolTip(newToolTip);
          valChanged = true;
        }
      }
      else if (
        (rawitem->isExpression() || !rawitem->isSet(elementIdx)) ||
        ditem->value(elementIdx) != editBox->text().toDouble())
      {
        ditem->setValue(elementIdx, editBox->text().toDouble());
        valChanged = true;
      }
    }
    else if (rawitem->type() == smtk::attribute::Item::IntType)
    {
      auto iitem = dynamic_pointer_cast<IntItem>(rawitem);
      if (
        (rawitem->isExpression() || !rawitem->isSet(elementIdx)) ||
        iitem->value(elementIdx) != editBox->text().toInt())
      {
        iitem->setValue(elementIdx, editBox->text().toInt());
        valChanged = true;
      }
    }
    else if (rawitem->type() == smtk::attribute::Item::StringType)
    {
      auto sitem = dynamic_pointer_cast<StringItem>(rawitem);
      if (
        (rawitem->isExpression() || !rawitem->isSet(elementIdx)) ||
        sitem->value(elementIdx) != editBox->text().toStdString())
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
  else if (
    textBox && !textBox->toPlainText().isEmpty() &&
    rawitem->type() == smtk::attribute::Item::StringType)
  {
    auto sitem = dynamic_pointer_cast<StringItem>(rawitem);
    if (
      (rawitem->isExpression() || !rawitem->isSet(elementIdx)) ||
      sitem->value(elementIdx) != textBox->toPlainText().toStdString())
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
  auto* iview = m_itemInfo.baseView();
  if (valChanged)
  {
    if (iview)
    {
      iview->valueChanged(rawitem->shared_from_this());
    }
    Q_EMIT this->modified();
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
  Q_EMIT this->modified();
}

bool qtInputsItem::isFixedWidth() const
{
  smtk::attribute::ValueItemPtr item = m_itemInfo.itemAs<ValueItem>();
  // If there is no item then its fixed width
  if (!item)
  {
    return true;
  }

  // If the item is discrete then its fixed width iff it have no children
  if (item->isDiscrete())
  {
    return (item->numberOfChildrenItems() == 0);
  }

  int widthValue = 1;
  m_itemInfo.component().attributeAsInt("FixedWidth", widthValue);
  // If the item has an explicit FixedWidth Attribute set to something
  // larger than 0 then its fixed width
  return (widthValue > 0);
}

Qt::Orientation qtInputsItem::getOrientation(const qtAttributeItemInfo& info)
{
  // Carries out logic listed above for setting internals member VectorItemOrient.
  auto valueItem = info.itemAs<smtk::attribute::ValueItem>();
  if (valueItem == nullptr) // safety first
  {
    return Qt::Horizontal;
  }

  // Extensible items are *always* vertical (cannot override)
  if (valueItem->isExtensible())
  {
    return Qt::Vertical;
  }

  // Check for "Layout" ItemView
  std::string layout;
  bool hasLayoutSpec = info.component().attribute("Layout", layout);
  if (hasLayoutSpec && layout == "Horizontal")
  {
    return Qt::Horizontal;
  }
  else if (hasLayoutSpec && layout == "Vertical")
  {
    return Qt::Vertical;
  }

  // For non-discrete item, default layout is horizontal
  if (!valueItem->isDiscrete())
  {
    return Qt::Horizontal;
  }

  // For discrete items, use horizontal layout (only) if
  //  * All children items have "blank" label (whitespace)
  //  * All discrete values have 0 or 1 child item

  // Check all children items for blank label.
  auto valueItemDef = std::dynamic_pointer_cast<const smtk::attribute::ValueItemDefinition>(
    info.item()->definition());
  const std::map<std::string, smtk::attribute::ItemDefinitionPtr>& childrenDefs =
    valueItemDef->childrenItemDefinitions();
  auto defIter = childrenDefs.begin();
  for (; defIter != childrenDefs.end(); ++defIter)
  {
    auto childDef = defIter->second;
    std::string label = childDef->label();
    std::string trimmed = smtk::common::StringUtil::trim(label);
    if (!trimmed.empty())
    {
      return Qt::Vertical;
    }
  } // for (defIter)

  // Next check if all discrete values have 0 or 1 child item.
  // If more than 1, use vertical layout.
  std::string ename;
  std::vector<std::string> childItems;
  std::size_t numValues = valueItemDef->numberOfDiscreteValues();
  for (std::size_t i = 0; i < numValues; ++i)
  {
    ename = valueItemDef->discreteEnum(i);
    childItems = valueItemDef->conditionalItems(ename);
    if (childItems.size() > 1)
    {
      return Qt::Vertical;
    }
  } // for (i)

  // If we reach this point, with all values having 0 or 1 child item with no label,
  // use horizontal layout
  return Qt::Horizontal;
}
