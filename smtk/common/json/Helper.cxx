//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/json/Helper.h"

#include <memory>

namespace smtk
{
namespace common
{

using UUID = smtk::common::UUID;
using IDHelper = Helper<UUID, UUID>;

namespace
{
thread_local std::unique_ptr<IDHelper> s_instance;
} // anonymous namespace

template<>
Helper<UUID, UUID>* Helper<UUID, UUID>::instance()
{
  return s_instance.get();
}

template<>
Helper<UUID, UUID>* Helper<UUID, UUID>::activate()
{
  if (s_instance)
  {
    throw std::logic_error("Nested link helpers are disallowed. Perhaps you forgot to deactivate?");
  }
  s_instance = std::make_unique<IDHelper>();
  return s_instance.get();
}

template<>
bool Helper<smtk::common::UUID, smtk::common::UUID>::deactivate()
{
  bool didRelease = !!s_instance;
  s_instance = nullptr;
  return didRelease;
}

} // namespace common
} // namespace smtk
