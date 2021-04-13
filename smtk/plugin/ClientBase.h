//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_plugin_ClientBase_h
#define __smtk_plugin_ClientBase_h

#include "smtk/CoreExports.h"

#include "smtk/SystemConfig.h"

#include <memory>

namespace smtk
{
namespace plugin
{
/// A base class for Plugin Clients.
class SMTKCORE_EXPORT ClientBase : public std::enable_shared_from_this<ClientBase>
{
public:
  virtual ~ClientBase();
};
} // namespace plugin
} // namespace smtk

#endif
