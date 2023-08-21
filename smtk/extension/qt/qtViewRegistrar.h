//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtViewRegistrar_h
#define smtk_extension_qt_qtViewRegistrar_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtManager.h"
#include "smtk/view/Manager.h"
#include "smtk/view/Registrar.h"

namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtViewRegistrar
{
public:
  using Dependencies = std::tuple<view::Registrar>;

  static void registerTo(const smtk::common::Managers::Ptr&);
  static void unregisterFrom(const smtk::common::Managers::Ptr&);

  static void registerTo(const smtk::extension::qtManager::Ptr&);
  static void unregisterFrom(const smtk::extension::qtManager::Ptr&);

  static void registerTo(const smtk::view::Manager::Ptr&);
  static void unregisterFrom(const smtk::view::Manager::Ptr&);
};
} // namespace extension
} // namespace smtk

#endif
