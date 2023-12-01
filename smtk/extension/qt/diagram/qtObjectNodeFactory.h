//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtObjectNodeFactory_h
#define smtk_extension_qtObjectNodeFactory_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"

#include "smtk/common/Factory.h"
#include "smtk/common/TypeName.h"

class QGraphicsItem;

namespace smtk
{
namespace extension
{

/// Arguments passed to the constructor of a qtBaseObjectNode.
using ObjectNodeInput = smtk::common::factory::
  Inputs<qtDiagramGenerator*, smtk::resource::PersistentObject*, QGraphicsItem*>;

/**\brief A factory for objects derived from qtObjectNode.
  */
class SMTKQTEXT_EXPORT qtObjectNodeFactory
  : public smtk::common::Factory<smtk::extension::qtBaseObjectNode, ObjectNodeInput>
{
};
} // namespace extension
} // namespace smtk

#endif
