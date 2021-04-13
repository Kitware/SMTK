//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_matplotlib_Registrar_h
#define __smtk_extension_matplotlib_Registrar_h

#include "smtk/extension/matplotlib/Exports.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/session/mesh/Registrar.h"

namespace smtk
{
namespace extension
{
namespace matplotlib
{
class SMTKMATPLOTLIBEXT_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<operation::Registrar, mesh::Registrar>;

  static void registerTo(const smtk::operation::Manager::Ptr&);
  static void unregisterFrom(const smtk::operation::Manager::Ptr&);
};
} // namespace matplotlib
} // namespace extension
} // namespace smtk

#endif
