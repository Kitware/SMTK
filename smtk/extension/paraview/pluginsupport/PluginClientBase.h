//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_paraview_pluginsupport_PluginClientBase_h
#define __smtk_extension_paraview_pluginsupport_PluginClientBase_h

#include "smtk/extension/paraview/pluginsupport/Exports.h"

#include "smtk/SystemConfig.h"

#include <memory>

namespace smtk
{
namespace extension
{
namespace paraview
{
/// A base class for Plugin Clients.
class SMTKPLUGINSUPPORT_EXPORT PluginClientBase
  : public std::enable_shared_from_this<PluginClientBase>
{
public:
  virtual ~PluginClientBase();
};
}
}
}

#endif
