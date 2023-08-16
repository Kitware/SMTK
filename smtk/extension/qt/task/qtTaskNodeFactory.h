//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtTaskNodeFactory_h
#define smtk_extension_qtTaskNodeFactory_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/task/qtBaseTaskNode.h"

#include "smtk/common/Factory.h"
#include "smtk/common/TypeName.h"

class QGraphicsItem;

namespace smtk
{
namespace extension
{
/**\brief A factory for qtTaskNodes
  */
class SMTKQTEXT_EXPORT qtTaskNodeFactory
  : public smtk::common::Factory<
      smtk::extension::qtBaseTaskNode,
      smtk::common::factory::Inputs<qtTaskScene*, smtk::task::Task*, QGraphicsItem*>>
{
};
} // namespace extension
} // namespace smtk

#endif
