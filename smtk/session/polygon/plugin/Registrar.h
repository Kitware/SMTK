//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_polygon_plugin_Registrar_h
#define smtk_session_polygon_plugin_Registrar_h

#include "smtk/session/polygon/Exports.h"

#include "smtk/view/Manager.h"

namespace smtk
{
namespace session
{
namespace polygon
{
namespace plugin
{

class SMTKPOLYGONSESSION_EXPORT Registrar
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
