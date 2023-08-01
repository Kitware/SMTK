//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtDoubleUnitsLineEdit.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/common/StringUtil.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QColor>
#include <QCompleter>
#include <QDebug>
#include <QFont>
#include <QKeyEvent>
#include <QStringListModel>
#include <QVariant>
#include <Qt>

#include "units/Measurement.h"

#include <algorithm> // std::sort et al
#include <sstream>

namespace
{
// Alias for split-string method
const auto& splitInput = smtk::attribute::DoubleItemDefinition::splitStringStartingDouble;

/** \brief Subclass QStringListModel to highlight first item */
class qtCompleterStringModel : public QStringListModel
{
public:
  qtCompleterStringModel(QObject* parent = nullptr)
    : QStringListModel(parent)
  {
  }

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
  {
    if (role == Qt::FontRole && index.row() == 0)
    {
      QVariant var = QStringListModel::data(index, role);
      QFont font = qvariant_cast<QFont>(var);
      font.setBold(true);
      return font;
    }
    // (else)
    return QStringListModel::data(index, role);
  }
};

} // anonymous namespace

namespace smtk
{
namespace extension
{
QWidget* qtDoubleUnitsLineEdit::checkAndCreate(qtInputsItem* inputsItem, const QString& tooltip)
{
  // Create qWarning object without string quoting
  QDebug qtWarning(QtWarningMsg);
  qtWarning.noquote();
  qtWarning.nospace();

  // Get the item definition and see if it specifies dimensional units
  auto dDef = inputsItem->item()->definitionAs<smtk::attribute::DoubleItemDefinition>();
  if (dDef->units().empty())
  {
    return nullptr;
  }

  // Sanity check that units only supported for numerical values
  if (dDef->isDiscrete())
  {
    qtWarning << "Ignoring units for discrete or expression item "
              << inputsItem->item()->name().c_str() << "\".";
    return nullptr;
  }

  // Get units system
  auto unitsSystem = dDef->unitsSystem();
  if (unitsSystem == nullptr)
  {
    return nullptr;
  }

  // Try parsing the unit string
  bool parsedOK = false;
  auto unit = unitsSystem->unit(dDef->units(), &parsedOK);
  if (!parsedOK)
  {
#ifndef NDEBUG
    qtWarning << "Ignoring unrecognized units \"" << dDef->units().c_str() << "\""
              << " in attribute item \"" << inputsItem->item()->name().c_str() << "\".";
#endif
    return nullptr;
  }

  auto* editor = new qtDoubleUnitsLineEdit(inputsItem, unit, tooltip);
  return static_cast<QWidget*>(editor);
}

qtDoubleUnitsLineEdit::qtDoubleUnitsLineEdit(
  qtInputsItem* item,
  const units::Unit& unit,
  const QString& tooltip)
  : QLineEdit(item->widget())
  , m_inputsItem(item)
  , m_unit(unit)
  , m_baseTooltip(tooltip)
{
  auto dDef = m_inputsItem->item()->definitionAs<smtk::attribute::DoubleItemDefinition>();
  // Set placeholder text
  this->setPlaceholderText(QString::fromStdString(dDef->units()));

  // Get list of compatible units
  auto compatibleUnits = m_unit.system()->compatibleUnits(m_unit);

  //  create a list of possible units names
  for (const auto& unit : compatibleUnits)
  {
    m_unitChoices.push_back(unit.name().c_str());
  }

  // Lets remove duplicates and sort the list
  m_unitChoices.removeDuplicates();
  m_unitChoices.sort();
  // Now make the Definition's units appear at the top
  m_unitChoices.push_front(dDef->units().c_str());

  // Instantiate completer with (empty) string list model
  auto* model = new qtCompleterStringModel(this);
  m_completer = new QCompleter(model, m_inputsItem->widget());
  m_completer->setCompletionMode(QCompleter::PopupCompletion);
  this->setCompleter(m_completer);
  QObject::connect(this, &QLineEdit::textEdited, this, &qtDoubleUnitsLineEdit::onTextEdited);
  //Add the converted value of the item to the tooltip if set
  auto dItem = std::dynamic_pointer_cast<smtk::attribute::DoubleItem>(m_inputsItem->item());
}

void qtDoubleUnitsLineEdit::onTextEdited()
{
  QPalette palette = this->palette();

  QString text = this->text();
  auto utext = text.toStdString();
  if (utext.empty())
  {
    QColor invalidColor = m_inputsItem->uiManager()->correctedInvalidValueColor();
    palette.setColor(QPalette::Base, invalidColor);
    this->setPalette(palette);
    return;
  }

  // Update the completer strings
  QStringList compatibleList;

  std::string valueString;
  std::string unitsString;
  bool ok = splitInput(utext, valueString, unitsString);
  if (ok)
  {
    if (unitsString.empty() || unitsString[0] == ' ')
    {
      valueString += ' ';
    }

    // Generate the completer strings
    for (const QString& unit : m_unitChoices)
    {
      QString entry(valueString.c_str());
      entry += unit;
      compatibleList << entry;
    } // for
  }   // if (ok)
  auto* model = dynamic_cast<qtCompleterStringModel*>(m_completer->model());
  model->setStringList(compatibleList);

  // Shouldn't need to call complete() but doesn't display without it
  m_completer->complete();

  // Update background based on current input string
  bool didParse = false;
  auto measurement = m_unit.system()->measurement(utext, &didParse);
  if (!didParse)
  {
    QColor invalidColor = m_inputsItem->uiManager()->correctedTempInvalidValueColor();
    palette.setColor(QPalette::Base, invalidColor);
    this->setPalette(palette);
    return;
  }

  bool inputHasUnits = !smtk::common::StringUtil::trim(unitsString).empty();
  bool conformal = measurement.m_units.dimension() == m_unit.dimension();
  if (!conformal && inputHasUnits)
  {
    QColor invalidColor = m_inputsItem->uiManager()->correctedInvalidValueColor().lighter(110);
    palette.setColor(QPalette::Base, invalidColor);
    this->setPalette(palette);
    return;
  }

  // Check if in range
  auto dDef = m_inputsItem->item()->definitionAs<smtk::attribute::DoubleItemDefinition>();
  if (dDef->hasRange())
  {
    bool converted = false;
    double convertedValue;
    if (!inputHasUnits)
    {
      std::istringstream iss(valueString);
      iss >> convertedValue;
      converted = !(iss.bad() || iss.fail());
    }
    else
    {
      units::Measurement convertedMsmt = m_unit.system()->convert(measurement, m_unit, &converted);
      if (!converted)
      {
        std::ostringstream ss;
        ss << "Failed to convert measurement: " << measurement << " to units: " << m_unit;
        qWarning() << ss.str().c_str();
      }
      else
      {
        convertedValue = convertedMsmt.m_value;
      } // else (input converts to measurement)
    }   // else (!inputHasUnits)

    if (!(converted && dDef->isValueValid(convertedValue)))
    {
      QColor invalidColor = m_inputsItem->uiManager()->correctedInvalidValueColor().lighter(110);
      palette.setColor(QPalette::Base, invalidColor);
      this->setPalette(palette);
      return;
    }
  } // if (def has range spec)

  palette.setColor(QPalette::Base, QColor("#ffffff"));
  this->setPalette(palette);
}

void qtDoubleUnitsLineEdit::onEditFinished()
{
  // This works around unexpected QLineEdit behavior. When certain keys
  // are pressed (backspace, e.g.), a QLineEdit::editingFinished signal
  // is emitted. The cause is currently unknown.
  // This code ignores the editingFinished signal if the editor is still
  // in focus and the last key pressed was not <Enter> or <Return>.
  bool finished = (m_lastKey == Qt::Key_Enter) || (m_lastKey == Qt::Key_Return);
  if (!finished && this->hasFocus())
  {
    qDebug() << "qtDoubleUnitsLineEdit::onEditFinished() ignoring key" << Qt::hex << Qt::showbase
             << m_lastKey;
    return;
  }

  // Check if we need to add units string
  std::string input = this->text().toStdString();
  std::string valueString;
  std::string unitsString;
  if (!splitInput(input, valueString, unitsString))
  {
    return;
  }

  // Yes - add (default) units string to the current line edit contents
  std::string trimmedString = smtk::common::StringUtil::trim(unitsString);
  if (trimmedString.empty())
  {
    auto dDef = m_inputsItem->item()->definitionAs<smtk::attribute::DoubleItemDefinition>();
    std::ostringstream ss;
    ss << valueString << ' ' << dDef->units();
    this->blockSignals(true);
    this->setText(QString::fromStdString(ss.str()));
    this->blockSignals(false);
  }
}

void qtDoubleUnitsLineEdit::keyPressEvent(QKeyEvent* event)
{
  // Save last key pressed
  m_lastKey = event->key();
  QLineEdit::keyPressEvent(event);
}

} // namespace extension
} // namespace smtk
