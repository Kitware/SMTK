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

#include <sstream>

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
    m_completer = new QCompleter(parentWidget);
  }
  else
  {
    QStringList compatibleList;
    m_compatibleUnits = unitsSystem->compatibleUnits(m_unit);
    for (const auto& u : m_compatibleUnits)
    {
      compatibleList << QString::fromStdString(u.name());
    }
    compatibleList.sort();
    m_completer = new QCompleter(compatibleList, parentWidget);
  }
  m_completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
  this->setCompleter(m_completer);
  QObject::connect(this, &QLineEdit::textChanged, this, &qtDoubleUnitsLineEdit::onTextChanged);
}

void qtDoubleUnitsLineEdit::onTextChanged(const QString& text)
{
  // qDebug() << "onTextChanged " << text;
  QPalette palette = this->palette();
  auto utext = text.toStdString();
  if (utext.empty())
  {
    palette.setColor(QPalette::Base, QColor("#ffffff"));
    this->setPalette(palette);
    return;
  }

  bool didParse = false;
  auto measurement = m_unitsSystem->measurement(utext, &didParse);
#ifndef NDEBUG
  std::stringstream ss;
  ss << measurement << " (" << measurement.m_units.dimension() << ')';
  qDebug() << ss.str().c_str();
#endif

  if (measurement.m_value != 0.)
  {
    QStringList compatibleUnitList;
    for (const auto& unit : m_compatibleUnits)
    {
      std::ostringstream prompt;
      prompt << measurement.m_value << " " << unit.name();
      compatibleUnitList << QString::fromStdString(prompt.str());
    }
    m_completer->setModel(new QStringListModel(compatibleUnitList, this->parentWidget()));
  }

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

  palette.setColor(QPalette::Base, QColor("#ffffff"));
  this->setPalette(palette);
}

} // namespace extension
} // namespace smtk
