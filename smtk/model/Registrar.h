//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_Registrar_h
#define smtk_model_Registrar_h

#include "smtk/CoreExports.h"

#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

namespace smtk
{
namespace model
{
class SMTKCORE_EXPORT Registrar
{
public:
  static void registerTo(const smtk::operation::Manager::Ptr&);
  static void unregisterFrom(const smtk::operation::Manager::Ptr&);

  static void registerTo(const smtk::resource::Manager::Ptr&);
  static void unregisterFrom(const smtk::resource::Manager::Ptr&);
};
} // namespace model
} // namespace smtk

#endif
