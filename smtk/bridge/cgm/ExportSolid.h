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

#include "smtk/bridge/cgm/cgmSMTKExports.h" // for CGMSMTK_EXPORT
#include "smtk/PublicPointerDefs.h" // For ManagerPtr

#include "smtk/common/UUID.h"

namespace smtk {
  namespace model {
    class Manager;
  }
}

namespace smtk {
  namespace bridge {
    namespace cgm {

/**\brief Load a solid model using CGM.
  *
  */
class CGMSMTK_EXPORT ExportSolid
{
public:
  static int entitiesToFileOfNameAndType(
    const std::vector<smtk::model::EntityRef>& entities,
    const std::string& filename,
    const std::string& filetype);
};

    } // namespace cgm
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_cgm_ExportSolid_h
