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

#include <QCompleter>
#include <QDebug>
#include <QFont>
#include <QStringListModel>
#include <QVariant>

#include "units/Measurement.h"

#include <algorithm> // std::sort et al
#include <sstream>

namespace
{
bool compareUnitNames(const units::Unit& a, const units::Unit& b)
{
  // For sorting units by name
  return a.name() < b.name();
}

/** \brief Splits input string into double value and any remaining part
 *
 * Returns true if double was found
 */
bool splitInput(const std::string& input, std::string& valueString, std::string& unitsString)
{
  valueString.clear();
  unitsString.clear();

  // Try streaming double value
  std::istringstream iss(input);
  double value;
  iss >> value;
  if ((iss.bad()) || iss.fail())
  {
    return false;
  }

  if (iss.eof())
  {
    valueString = input;
    return true;
  }

  std::size_t pos = iss.tellg();
  valueString = input.substr(0, pos);
  unitsString = input.substr(pos);

  return true;
}

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
QWidget* qtDoubleUnitsLineEdit::checkAndCreate(
  smtk::attribute::ConstDoubleItemPtr item,
  QWidget* parent)
{
  // Create qWarning object without string quoting
  QDebug qtWarning(QtWarningMsg);
  qtWarning.noquote();
  qtWarning.nospace();

  // Get the item definition and see if it specifies dimensional units
  auto dDef = dynamic_pointer_cast<const smtk::attribute::DoubleItemDefinition>(item->definition());
  if (dDef->units().empty())
  {
    return nullptr;
  }

  // Sanity check that units only supported for numerical values
  if (dDef->isDiscrete() || dDef->allowsExpressions())
  {
    qtWarning << "Ignoring units for discrete or expression item " << item->name().c_str() << "\".";
    return nullptr;
  }

  // Get units system
  auto unitsSystem = item->attribute()->attributeResource()->unitsSystem();
  if (unitsSystem == nullptr)
  {
    return nullptr;
  }

  // Try parsing the unit string
  bool parsedOK = false;
  auto unit = unitsSystem->unit(dDef->units(), &parsedOK);
  if (!parsedOK || unit.dimensionless())
  {
#ifndef NDEBUG
    qtWarning << "Ignoring unrecognized units \"" << dDef->units().c_str() << "\""
              << " in attribute item \"" << item->name().c_str() << "\".";
#endif
    return nullptr;
  }

  auto* editor = new qtDoubleUnitsLineEdit(dDef, unit, parent);
  return static_cast<QWidget*>(editor);
}

qtDoubleUnitsLineEdit::qtDoubleUnitsLineEdit(
  smtk::attribute::ConstDoubleItemDefinitionPtr def,
  const units::Unit& unit,
  QWidget* parentWidget)
  : QLineEdit(parentWidget)
  , m_def(def)
  , m_unit(unit)
{
  // Set placeholder text
  this->setPlaceholderText(QString::fromStdString(unit.name()));

  // Get list of compatible units
  m_compatibleUnits = m_unit.system()->compatibleUnits(m_unit);
  std::sort(m_compatibleUnits.begin(), m_compatibleUnits.end(), compareUnitNames);

  // Find the same unit in the list and move it to front of list
  auto matchUnit = [this](const units::Unit& u) { return u == m_unit; };
  auto iter = std::find_if(m_compatibleUnits.begin(), m_compatibleUnits.end(), matchUnit);
  if (iter != m_compatibleUnits.end())
  {
    std::rotate(m_compatibleUnits.begin(), iter, iter + 1);
  }

  // Instantiate completer with (empty) string list model
  auto* model = new qtCompleterStringModel(this);
  m_completer = new QCompleter(model, parentWidget);
  m_completer->setCompletionMode(QCompleter::PopupCompletion);
  this->setCompleter(m_completer);
  QObject::connect(this, &QLineEdit::textEdited, this, &qtDoubleUnitsLineEdit::onTextEdited);
}

void qtDoubleUnitsLineEdit::onTextEdited()
{
  QPalette palette = this->palette();

  QString text = this->text();
  auto utext = text.toStdString();
  if (utext.empty())
  {
    palette.setColor(QPalette::Base, QColor("#ffffff"));
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
    std::ostringstream prompt;
    for (const auto& unit : m_compatibleUnits)
    {
      prompt.str("");
      prompt.clear();
      prompt << valueString << unit.name();
      compatibleList << QString::fromStdString(prompt.str());
    } // for
    // qDebug() << compatibleList;
  } // if (ok)
  auto* model = dynamic_cast<qtCompleterStringModel*>(m_completer->model());
  model->setStringList(compatibleList);

  // Shouldn't need to call complete() but doesn't display without it
  m_completer->complete();

  bool didParse = false;
  auto measurement = m_unit.system()->measurement(utext, &didParse);
  if (!didParse)
  {
    palette.setColor(QPalette::Base, QColor("#ffb9b9"));
    this->setPalette(palette);
    return;
  }

  bool conformal = measurement.m_units.dimension() == m_unit.dimension();
  if (!conformal)
  {
    palette.setColor(QPalette::Base, QColor("#ffdab9"));
    this->setPalette(palette);
    return;
  }

  // Check if in range
  if (m_def->hasRange())
  {
    bool converted = false;
    units::Measurement convertedMsmt = m_unit.system()->convert(measurement, m_unit, &converted);
    if (!converted)
    {
      std::ostringstream ss;
      ss << "Failed to convert measurement: " << measurement << " to units: " << m_unit;
      qWarning() << ss.str().c_str();
    }
    else
    {
      double convertedValue = convertedMsmt.m_value;
      if (!m_def->isValueValid(convertedValue))
      {
        palette.setColor(QPalette::Base, QColor("#ffb9b9"));
        this->setPalette(palette);
        return;
      }
    }
  }

  palette.setColor(QPalette::Base, QColor("#ffffff"));
  this->setPalette(palette);
}

void qtDoubleUnitsLineEdit::onEditFinished()
{
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
    std::ostringstream ss;
    ss << valueString << ' ' << m_unit.name();
    this->blockSignals(true);
    this->setText(QString::fromStdString(ss.str()));
    this->blockSignals(false);
  }
}

} // namespace extension
} // namespace smtk
