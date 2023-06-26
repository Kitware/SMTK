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
  , m_unit(unitsSystem->unit(def->units()))
{
}

} // namespace extension
} // namespace smtk
