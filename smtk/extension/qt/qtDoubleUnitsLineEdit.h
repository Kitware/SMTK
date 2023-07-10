//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtDoubleUnitsLineEdit_h
#define smtk_extension_qt_qtDoubleUnitsLineEdit_h

#include <QLineEdit>

#include "smtk/extension/qt/Exports.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/qtInputsItem.h"

#include <QPointer>
#include <QString>
#include <QStringList>

#include "units/System.h"
#include "units/Unit.h"

#include <vector>

class QCompleter;

namespace smtk
{
namespace extension
{
class qtInputsItem;

/**\brief qtDoubleUnitsLineEdit provides units-aware line edit double values */
class SMTKQTEXT_EXPORT qtDoubleUnitsLineEdit : public QLineEdit
{
  Q_OBJECT
public:
  using Superclass = QLineEdit;

  /** \brief Creates instance if double item has units; Returns editor as QWidget */
  static QWidget* checkAndCreate(qtInputsItem* item);
  qtDoubleUnitsLineEdit(qtInputsItem* item, const units::Unit& unit);
  ~qtDoubleUnitsLineEdit() override = default;

Q_SIGNALS:

public Q_SLOTS:
  void onEditFinished();

protected Q_SLOTS:
  void onTextEdited();

protected:
  QPointer<qtInputsItem> m_inputsItem;
  units::Unit m_unit;
  std::vector<units::Unit> m_compatibleUnits;

  QCompleter* m_completer = nullptr;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qt_qtDoubleUnitsLineEdit_h
