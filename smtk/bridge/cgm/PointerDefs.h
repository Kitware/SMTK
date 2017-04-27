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

#ifndef __smtk_session_cgm_PointerDefs_h
#define __smtk_session_cgm_PointerDefs_h

#include "smtk/SharedPtr.h"
#include "smtk/SystemConfig.h"

namespace smtk
{
namespace bridge
{
namespace cgm
{

class Session;
typedef smtk::shared_ptr<smtk::bridge::cgm::Session> SessionPtr;
class ImportSolid;
class Engines;
class ExportSolid;

} // namespace cgm
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_cgm_PointerDefs_h
