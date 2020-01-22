//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vxl_operators_Registrar_h
#define smtk_extension_vxl_operators_Registrar_h

#include "smtk/extension/vxl/operators/Exports.h"

#include "smtk/view/Manager.h"

namespace smtk
{
namespace extension
{
namespace vxl
{
namespace operators
{

class SMTKVXLOPERATIONVIEWSEXT_EXPORT Registrar
{
public:
  static void registerTo(const smtk::view::Manager::Ptr&);
  static void unregisterFrom(const smtk::view::Manager::Ptr&);
};
}
}
}
}

#endif
