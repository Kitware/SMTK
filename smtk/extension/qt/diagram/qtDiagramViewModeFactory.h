//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtDiagramViewModeFactory_h
#define smtk_extension_qtDiagramViewModeFactory_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/diagram/qtDiagramViewMode.h"

#include "smtk/common/Factory.h"
#include "smtk/common/TypeName.h"

class QActionGroup;
class QToolBar;

namespace smtk
{
namespace extension
{

class qtDiagramView;

/// Arguments passed to the constructor of a qtDiagramViewMode.
using DiagramViewModeInput =
  smtk::common::factory::Inputs<qtDiagram*, qtDiagramView*, QToolBar*, QActionGroup*>;

/**\brief A factory for qtDiagramViewModes
  */
class SMTKQTEXT_EXPORT qtDiagramViewModeFactory
  : public smtk::common::Factory<smtk::extension::qtDiagramViewMode, DiagramViewModeInput>
{
};
} // namespace extension
} // namespace smtk

#endif
