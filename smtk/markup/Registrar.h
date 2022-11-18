//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Registrar_h
#define smtk_markup_Registrar_h

#include "smtk/markup/Exports.h"

#include "smtk/attribute/Registrar.h"
#include "smtk/common/Managers.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/resource/Manager.h"
#include "smtk/view/Manager.h"
#include "smtk/view/Registrar.h"

namespace smtk
{
namespace markup
{

class SMTKMARKUP_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<operation::Registrar, attribute::Registrar, view::Registrar>;

  static void registerTo(const smtk::common::Managers::Ptr&);
  static void unregisterFrom(const smtk::common::Managers::Ptr&);

  static void registerTo(const smtk::operation::Manager::Ptr&);
  static void unregisterFrom(const smtk::operation::Manager::Ptr&);

  static void registerTo(const smtk::resource::Manager::Ptr&);
  static void unregisterFrom(const smtk::resource::Manager::Ptr&);

  static void registerTo(const smtk::view::Manager::Ptr& viewManager);
  static void unregisterFrom(const smtk::view::Manager::Ptr& viewManager);
};

} // namespace markup
} // namespace smtk

#endif
