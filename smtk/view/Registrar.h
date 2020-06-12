//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_view_Registrar_h
#define __smtk_view_Registrar_h

#include "smtk/CoreExports.h"

#include "smtk/view/Manager.h"

namespace smtk
{
namespace view
{
class SMTKCORE_EXPORT Registrar
{
public:
  static void registerTo(const smtk::view::Manager::Ptr&);
  static void unregisterFrom(const smtk::view::Manager::Ptr&);
};
}
}

#endif
