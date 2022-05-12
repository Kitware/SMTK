//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_markup_DomainFactory_h
#define smtk_markup_DomainFactory_h

#include "smtk/markup/Component.h"

#include "smtk/common/Factory.h"
#include "smtk/common/UUID.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace markup
{

class Resource;

using DomainFactory = smtk::common::Factory<
  smtk::markup::Domain,
  // Possible constructor arguments:
  // 0. Nothing:
  void,
  // 1. Just a token (name):
  smtk::string::Token,
  // 2. A JSON object
  const nlohmann::json&>;

} // namespace markup
} // namespace smtk

#endif // smtk_markup_DomainFactory_h
