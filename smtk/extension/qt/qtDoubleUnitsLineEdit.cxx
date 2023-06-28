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

#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/common/StringUtil.h"

#include <QCompleter>
#include <QDebug>
#include <QStringListModel>

#include "units/Measurement.h"

#include <algorithm> // std::sort
#include <iostream>
#include <sstream>

namespace
{
bool compareUnitNames(const units::Unit& a, const units::Unit& b)
{
  // For sorting units by name
  return a.name() < b.name();
}
} // namespace

namespace smtk
{
namespace extension
{

qtDoubleUnitsLineEdit::qtDoubleUnitsLineEdit(
  smtk::attribute::ConstDoubleItemDefinitionPtr def,
  std::shared_ptr<units::System> unitsSystem,
  QWidget* parentWidget)
  : QLineEdit(parentWidget)
  , m_def(def)
  , m_unitsSystem(unitsSystem)
{
  std::string unitsString = def->units();
  std::string trimUnitsString = smtk::common::StringUtil::trim(unitsString);
  bool success = false;
  m_unit = unitsSystem->unit(trimUnitsString, &success);
  if (!success)
  {
    qWarning() << "Unable to parse unit string" << unitsString.c_str();
    unitsString.clear();
  }
  this->setPlaceholderText(QString::fromStdString(unitsString));

  if (unitsString.empty())
  {
    qCritical() << "Underlying definition MUST have units defined";
  }

  // Initialize list of compatible units
  m_compatibleUnits = unitsSystem->compatibleUnits(m_unit);
  std::sort(m_compatibleUnits.begin(), m_compatibleUnits.end(), compareUnitNames);

  // Find the same unit in the list and move it to front of list
  auto matchUnit = [this](const units::Unit& u) { return u == m_unit; };
  auto iter = std::find_if(m_compatibleUnits.begin(), m_compatibleUnits.end(), matchUnit);
  if (iter != m_compatibleUnits.end())
  {
    std::rotate(m_compatibleUnits.begin(), iter, iter + 1);
  }

  // Instantiate completer with (empty) string list model
  QStringList list;
  m_completer = new QCompleter(list, parentWidget);
  m_completer->setCompletionMode(QCompleter::PopupCompletion);
  this->setCompleter(m_completer);
}

void qtDoubleUnitsLineEdit::onTextChanged()
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

  // Parse the text
  bool didParse = false;
  auto measurement = m_unitsSystem->measurement(utext, &didParse);

  // Generate completer list with current value
  QStringList compatibleList;
  if (measurement.m_value != 0.)
  {
    for (const auto& unit : m_compatibleUnits)
    {
      std::ostringstream prompt;
      std::string uname = unit.name();
      prompt << measurement.m_value << " " << uname;
      compatibleList << QString::fromStdString(prompt.str());
    } // for
  }

  auto* model = qobject_cast<QStringListModel*>(m_completer->model());
  model->setStringList(compatibleList);

  // Shouldn't need to call complete() but doesn't display without it
  m_completer->complete();

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
    units::Measurement convertedMsmt = m_unitsSystem->convert(measurement, m_unit, &converted);
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
#if 0
  // std::string valString = this->text().toStdString();
  // bool success = false;
  // auto valMeasure = m_unitsSystem->measurement(valString, &success);
  // if (success && valMeasure.m_units.dimensionless())
  // {
  //   std::ostringstream ss;
  //   ss << valString << ' ' << m_unit;
  //   this->blockSignals(true);
  //   this->setText(QString::fromStdString(ss.str()));
  //   this->blockSignals(false);
  // }
#else
  // Check for space in middle of text, even though not required by units library.
  // Using this approach because we don't know how to tell the difference
  // between an invalid units e.g., "3x" and missing units (both are dimensionless).
  QString text = this->text().trimmed();
  if (text.indexOf(' ') < 0)
  {
    std::ostringstream ss;
    ss << text.toStdString() << ' ' << m_unit.name();
    this->blockSignals(true);
    this->setText(QString::fromStdString(ss.str()));
    this->blockSignals(false);
  }
#endif
}

} // namespace extension
} // namespace smtk
