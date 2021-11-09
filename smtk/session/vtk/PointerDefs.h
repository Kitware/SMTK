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

#ifndef smtk_session_vtk_PointerDefs_h
#define smtk_session_vtk_PointerDefs_h

#include "smtk/SharedPtr.h"
#include "smtk/SystemConfig.h"

namespace smtk
{ //session relates pointer classes
namespace session
{
namespace vtk
{
class Session;
typedef smtk::shared_ptr<smtk::session::vtk::Session> SessionPtr;
struct EntityHandle;
} // namespace vtk
} // namespace session
} // namespace smtk
#endif
