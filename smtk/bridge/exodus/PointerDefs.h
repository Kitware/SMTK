//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME PublicPointerDefs.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_session_exodus_PointerDefs_h
#define __smtk_session_exodus_PointerDefs_h

#include "smtk/SharedPtr.h"
#include "smtk/SystemConfig.h"

namespace smtk
{ //session relates pointer classes
namespace bridge
{
namespace exodus
{
class Session;
typedef smtk::shared_ptr<smtk::bridge::exodus::Session> SessionPtr;
struct EntityHandle;
}
}
}
#endif
