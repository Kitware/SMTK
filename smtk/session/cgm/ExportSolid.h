//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_cgm_ExportSolid_h
#define __smtk_session_cgm_ExportSolid_h

#include "smtk/PublicPointerDefs.h"   // For ManagerPtr
#include "smtk/session/cgm/Exports.h" // for SMTKCGMSESSION_EXPORT

#include "smtk/common/UUID.h"

namespace smtk
{
namespace model
{
class Manager;
}
}

namespace smtk
{
namespace session
{
namespace cgm
{

/**\brief Load a solid model using CGM.
  *
  */
class SMTKCGMSESSION_EXPORT ExportSolid
{
public:
  static int entitiesToFileOfNameAndType(const std::vector<smtk::model::EntityRef>& entities,
    const std::string& filename, const std::string& filetype);
};

} // namespace cgm
} // namespace session
} // namespace smtk

#endif // __smtk_session_cgm_ExportSolid_h
