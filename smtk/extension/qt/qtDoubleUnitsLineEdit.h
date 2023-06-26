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

#include "units/System.h"
#include "units/Unit.h"

namespace smtk
{
namespace extension
{
/**\brief qtDoubleUnitsLineEdit provides units-aware field for editing double values */
class SMTKQTEXT_EXPORT qtDoubleUnitsLineEdit : public QLineEdit
{
  Q_OBJECT
public:
  using Superclass = QLineEdit;

  qtDoubleUnitsLineEdit(
    smtk::attribute::ConstDoubleItemDefinitionPtr def,
    std::shared_ptr<units::System> unitsSystem,
    QWidget* parent = nullptr);
  ~qtDoubleUnitsLineEdit() = default;

Q_SIGNALS:

public Q_SLOTS:

protected Q_SLOTS:

protected:
  smtk::attribute::ConstDoubleItemDefinitionPtr m_def;
  std::shared_ptr<units::System> m_unitsSystem;
  units::Unit m_unit;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qt_qtDoubleUnitsLineEdit_h
