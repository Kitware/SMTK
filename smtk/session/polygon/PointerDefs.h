//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef smtk_session_polygon_PointerDefs_h
#define smtk_session_polygon_PointerDefs_h

#include "smtk/SharedPtr.h"
#include "smtk/SystemConfig.h"

namespace smtk
{
namespace session
{
namespace polygon
{

class Session;
typedef smtk::shared_ptr<smtk::session::polygon::Session> SessionPtr;

} // namespace polygon
} // namespace session
} // namespace smtk

#endif // smtk_session_polygon_PointerDefs_h
