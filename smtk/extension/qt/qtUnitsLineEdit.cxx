//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtUnitsLineEdit.h"

#include "smtk/common/StringUtil.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QColor>
#include <QCompleter>
#include <QDebug>
#include <QFocusEvent>
#include <QFont>
#include <QKeyEvent>
#include <QRegularExpressionValidator>
#include <QStringListModel>
#include <QVariant>
#include <Qt>

#include "units/Converter.h"
#include "units/PreferredUnits.h"

#include <algorithm> // std::sort et al
#include <sstream>

using namespace smtk::attribute;
using namespace smtk::extension;

namespace
{

/** \brief Subclass QStringListModel to highlight first item */
class qtCompleterStringModel : public QStringListModel
{
public:
  qtCompleterStringModel(QObject* parent = nullptr)
    : QStringListModel(parent)
  {
  }

  [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
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

qtUnitsLineEdit::qtUnitsLineEdit(
  const QString& baseUnit,
  const std::shared_ptr<units::System>& unitSys,
  qtUIManager* uiManager,
  QWidget* parent)
  : QLineEdit(parent)
  , m_baseUnit(baseUnit)
  , m_unitSys(unitSys)
  , m_uiManager(uiManager)
{

  QStringList unitChoices;
  std::shared_ptr<units::PreferredUnits> preferred;
  auto cit = m_unitSys->m_unitContexts.find(m_unitSys->m_activeUnitContext);
  if (cit != m_unitSys->m_unitContexts.end())
  {
    preferred = cit->second;
  }

  // if the base unit is empty then all supported units are allowed
  if (m_baseUnit.isEmpty())
  {
    // Create a list of possible units names:
    if (preferred)
    {
      // We have a context; this provides preferred units as a
      // developer-ordered list of units, placing dUnits at
      // position 0 of the suggestions.
      units::CompatibleUnitOptions opts;
      opts.m_inputUnitPriority = 0;
      for (const auto& suggestion : preferred->suggestedUnits("*", opts))
      {
        unitChoices.push_back(suggestion.c_str());
      }
    }
    else
    {
      // We don't have a unit-system context; just
      // find compatible units. This may present
      // duplicates and is definitely not sorted.

      // Get list of compatible units
      auto compatibleUnits = m_unitSys->allUnits();
      for (const auto& unitName : compatibleUnits)
      {
        unitChoices.push_back(unitName.c_str());
      }
      // Lets remove duplicates and sort the list
      unitChoices.removeDuplicates();
      unitChoices.sort();
      // Now make the Item's units appear at the top
      //unitChoices.push_front(dUnits.c_str());
    }
  }
  else
  {
    // Parsing the unit string
    bool parsedOK = false;
    auto unit = m_unitSys->unit(m_baseUnit.toStdString(), &parsedOK);

    // If the units are supported then setup a completer
    if (parsedOK)
    {

      // Create a list of possible units names:
      if (preferred)
      {
        // We have a context; this provides preferred units as a
        // developer-ordered list of units, placing dUnits at
        // position 0 of the suggestions.
        units::CompatibleUnitOptions opts;
        opts.m_inputUnitPriority = 0;
        for (const auto& suggestion : preferred->suggestedUnits(m_baseUnit.toStdString(), opts))
        {
          unitChoices.push_back(suggestion.c_str());
        }
      }
      else
      {
        // We don't have a unit-system context; just
        // find compatible units. This may present
        // duplicates and is definitely not sorted.

        // Get list of compatible units
        auto compatibleUnits = m_unitSys->compatibleUnits(unit);
        for (const auto& unit : compatibleUnits)
        {
          unitChoices.push_back(unit.name().c_str());
        }
        // Lets remove duplicates and sort the list
        unitChoices.removeDuplicates();
        unitChoices.sort();
        // Now make the Item's units appear at the top
        //unitChoices.push_front(dUnits.c_str());
      }
    }
  }

  if (!unitChoices.empty())
  {
    auto* model = new qtCompleterStringModel(this);
    model->setStringList(unitChoices);
    m_completer = new QCompleter(model, parent);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    this->setCompleter(m_completer);
  }

  // Connect up the signals
  QObject::connect(this, &QLineEdit::textEdited, this, &qtUnitsLineEdit::onTextEdited);
  QObject::connect(this, &QLineEdit::editingFinished, this, &qtUnitsLineEdit::onEditFinished);

  // Prevent leading spaces
  this->setValidator(new QRegularExpressionValidator(QRegularExpression("^\\S*$"), this));
}

bool qtUnitsLineEdit::isCurrentTextValid() const
{
  auto utext = this->text().toStdString();
  // Remove all surrounding white space
  smtk::common::StringUtil::trim(utext);

  // Empty strings are only allowed if the baseUnit is also empty
  // which indicates that any supported units as well as unit-less
  // values are permitted
  if (utext.empty())
  {
    return m_baseUnit.isEmpty();
  }
  bool parsedOK = false;
  // Is the current value supported by the unit system?
  parsedOK = false;
  auto newU = m_unitSys->unit(utext, &parsedOK);
  if (!parsedOK)
  {
    return false;
  }

  if (m_baseUnit.isEmpty())
  {
    return true;
  }

  // Can you convert between the units?
  auto baseU = m_unitSys->unit(m_baseUnit.toStdString(), &parsedOK);
  return (m_unitSys->convert(baseU, newU) != nullptr);
}

void qtUnitsLineEdit::setAndClassifyText(const QString& newVal)
{
  this->setText(newVal);
  this->onTextEdited();
}

void qtUnitsLineEdit::onTextEdited()
{
  if (!this->isCurrentTextValid())
  {
    m_uiManager->setWidgetColorToInvalid(this);
    return;
  }
  if (this->text() == m_baseUnit)
  {
    m_uiManager->setWidgetColorToDefault(this);
    return;
  }
  m_uiManager->setWidgetColorToNormal(this);
}

void qtUnitsLineEdit::onEditFinished()
{
  // This works around unexpected QLineEdit behavior. When certain keys
  // are pressed (backspace, e.g.), a QLineEdit::editingFinished signal
  // is emitted. The cause is currently unknown.
  // This code ignores the editingFinished signal if the editor is still
  // in focus and the last key pressed was not <Enter> or <Return>.
  bool finished = (m_lastKey == Qt::Key_Enter) || (m_lastKey == Qt::Key_Return);
  if (!finished && this->hasFocus())
  {
    return;
  }

  // Don't emit completion if the current string is invalid
  if (this->isCurrentTextValid())
  {
    Q_EMIT this->editingCompleted(this->text());
  }
}

void qtUnitsLineEdit::keyPressEvent(QKeyEvent* event)
{
  // Save last key pressed
  m_lastKey = event->key();
  QLineEdit::keyPressEvent(event);
}

} // namespace extension
} // namespace smtk
