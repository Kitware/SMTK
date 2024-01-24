//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtDiagramGeneratorFactory_h
#define smtk_extension_qtDiagramGeneratorFactory_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"

#include "smtk/common/Factory.h"
#include "smtk/common/TypeName.h"

class QGraphicsItem;

namespace smtk
{
namespace extension
{

/// Arguments passed to the constructor of a qtDiagramGenerator.
using DiagramGeneratorInput = smtk::common::factory::
  Inputs<const smtk::view::Information&, const smtk::view::Configuration::Component&, qtDiagram*>;

/**\brief A factory for qtDiagramGenerator.
  */
class SMTKQTEXT_EXPORT qtDiagramGeneratorFactory
  : public smtk::common::Factory<smtk::extension::qtDiagramGenerator, DiagramGeneratorInput>
{
};
} // namespace extension
} // namespace smtk

#endif
